#include "MainRenderer.hpp"
#include <Graphics/Graphics.hpp>
#include "Graphics/Pipelines/PipelineGraphics.hpp"
#include "Maths/Vector3.hpp"
#include "Models/Model.hpp"
#include "Graphics/Buffers/UniformHandler.hpp"
#include "Graphics/Descriptors/DescriptorsHandler.hpp"
#include "Maths/Matrix4.hpp"
#include "Devices/Window.hpp"

namespace test
{
	/*
	 * Create something to render
	 * It will have a pipeline graphics and shaders
	 * We disable Depth tests
	 */
	class Tutorial6 : public Subrender
	{
	public:
		struct UniformBufferObject
		{
			acid::Matrix4 model;
			acid::Matrix4 view;
			acid::Matrix4 proj;
		} ubo;

		struct Vertex
		{
			Vector3f m_position;
			Vector3f color;
			Vector2f m_uv;
		};

	protected:
		/*
		 * We need to describe how these are input to the shader
		 *
		 * from the shader:
		 *	layout(location = 0) in vec2 inPosition;
		 *	layout(location = 1) in vec3 inColor;
		 *	layout(location = 2) in vec2 inTexCoord;
		 *
		 */
		const std::vector<Shader::VertexInput> vertexInputs = {
			{
				{{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}},
				{
					{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_position)},
					{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},
					{2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, m_uv)},
				}
			},
		};

		PipelineGraphics m_pipeline;
		std::unique_ptr<Model> m_Model;
		UniformHandler m_uniformScene;
		std::chrono::high_resolution_clock::time_point m_startTime;
		DescriptorsHandler m_descriptorSet;

		std::unique_ptr<Image2d> m_Image;

		/*
		 * To follow the Vulkan tutorial, we have flipped the Y in the projection, and here
		 * we are also reversing the triangle
		 * cull mode as per https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
		 *
		 * We changed the Depth Handling here to PipelineGraphics::Depth::ReadWrite to use the depth
		 * buffer
		 */
	public:
		explicit Tutorial6(const Pipeline::Stage &stage)
			: Subrender(stage)
			  , m_pipeline(stage,
				  {"Tutorial/Shaders/tri6.vert", "Tutorial/Shaders/tri6.frag"},
				  vertexInputs,
				  {}, PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::ReadWrite,
				  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL,
				  VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
			  , m_descriptorSet(m_pipeline)

		{
		}

		/*
		 * To render this, we just Bind the pipeline and draw.  Acid will call this at the right time.
		 */
		virtual void Render(const CommandBuffer &commandBuffer) override
		{
			/*
			 * Set the pipeline
			 */
			m_pipeline.BindPipeline(commandBuffer);

			/*
			 * Now to calculate the transform of the model
			 */
			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_startTime).count();

			ubo.model = Matrix4().Rotate(time * Maths::Radians(90.0f), Vector3f(0.0f, 0.0f, 1.0f));

			/*
			 * These have to match the order in the shader
			 */
			m_uniformScene.Push("model", ubo.model);
			m_uniformScene.Push("view", ubo.view);
			m_uniformScene.Push("proj", ubo.proj);

			/*
			 * This has to match the uniform setting in the shader
			 */
			m_descriptorSet.Push("UniformBufferObject", m_uniformScene);

			/*
			 * Acid takes care of all complexities of Vulkan here and we can just push the image to the
			 * sampler name in the shader
			 */
			m_descriptorSet.Push("texSampler", m_Image);

			/*
			 * Set it to the pipeline
			 */
			m_descriptorSet.Update(m_pipeline);
			m_descriptorSet.BindDescriptor(commandBuffer, m_pipeline);

			/*
			 * Draw the model, this time it knows it has a indexbuffer and does the indexed draw
			 * It will also pass the UBO to the shader
			 */
			(void)m_Model->CmdRender(commandBuffer);
		};

		void SetModel(std::unique_ptr<Model> &model)
		{
			m_Model = std::move(model);

			m_startTime = std::chrono::high_resolution_clock::now();

			/*
			 * Lets set up the camera viewpoint
			 * for now the models transform will be the Unit transform
			 */
			ubo.view = Matrix4::LookAt(Vector3f(2.0f, 2.0f, 2.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f));
			ubo.proj = Matrix4::PerspectiveMatrix(Maths::Radians(45.0f), Window::Get()->GetAspectRatio(), 0.1f, 10.0f);
			ubo.proj[1][1] *= -1;
			ubo.model = Matrix4();

			/*
			 * Lets create an image to be the texture of the object, we will load from the resources
			 * system
			 */
			m_Image = std::make_unique<Image2d>("Tutorial/texture.jpg");
		}
	};

	/*
	 * Application main renderer
	 * Create a single SubPass and define the only attachment to be the swapchain attachment
	 * The format doesnt matter since it is always matched to the Framebuffer/Window
	 * We set the clear color to Aqua and disable MSAA for now.
	 */
	MainRenderer::MainRenderer()
	{
		/*
		 * Define the attachments for the Pass
		 * We are adding a depth buffer to the list of attachments
		 */
		std::vector<Attachment> renderpassAttachments1{
			{0, "depth", Attachment::Type::Depth, false},
			{1, "swapchain", Attachment::Type::Swapchain, false, VK_FORMAT_UNDEFINED, Colour::Aqua},
		};

		/*
		 * Add the references to the attachments for the SubPass
		 * In our case, SubPass 1 will be using Attachment 1
		 * We have added the depth buffer to the list of attachments that the SubPass will
		 * since our pipeline uses it now
		 */
		std::vector<SubpassType> renderpassSubpasses1 = {{0, {0,1}}};
		AddRenderStage(std::make_unique<RenderStage>(renderpassAttachments1, renderpassSubpasses1));
	}

	void MainRenderer::Start()
	{
		/**
		 * Lets create a set of vertices and matching colors, a square this time
		 */
		const std::vector<Tutorial6::Vertex> vertices = {
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		};

		/*
		 * Create a set of indices
		 */
		const std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};

		/*
		 * Create a model from that set of Vertices and indices
		 */
		std::unique_ptr<Model> model = std::make_unique<Model>(vertices, indices);

		/*
		 * Ok set our square to be rendered in Pass 1, SubPass 1 - Acid will ask us to draw it
		 * Also provide how the buffers are mapped to the shader
		 */
		auto Square = AddSubrender<Tutorial6>({0, 0});

		/*
		 * Set the model
		 */
		Square->SetModel(model);
	}

	/*
	 * No updates need for this
	 */
	void MainRenderer::Update()
	{
	}
}
