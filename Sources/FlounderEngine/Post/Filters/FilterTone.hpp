#pragma once

#include "Post/IPostFilter.hpp"

namespace fl
{
	class F_EXPORT FilterTone :
		public IPostFilter
	{
	public:
		FilterTone(const GraphicsStage &graphicsStage);

		~FilterTone();

		void Render(const VkCommandBuffer &commandBuffer) override;
	};
}
