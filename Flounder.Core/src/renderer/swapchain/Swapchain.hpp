﻿#pragma once

#include <array>
#include <vector>

#include "../../platforms/glfw/GlfwVulkan.hpp"

namespace Flounder
{
	class Swapchain
	{
	private:
		VkPresentModeKHR m_presentMode;
		VkSwapchainKHR m_swapchain;
		uint32_t m_swapchainImageCount;
		std::vector<VkImage> m_swapchinImages;
		std::vector<VkImageView> m_swapchinImageViews;

		std::vector<VkFramebuffer> m_framebuffers;
	public:
		Swapchain();

		~Swapchain();

		void Create(const VkDevice &device, const VkPhysicalDevice &physicalDevice, const VkSurfaceKHR &surface, const VkSurfaceCapabilitiesKHR &surfaceCapabilities, const VkSurfaceFormatKHR &surfaceFormat, const VkExtent2D &extent);

		void CreateFrameBuffers(const VkDevice &device, const VkRenderPass &renderPass, const VkImageView &depthImageView, const VkExtent2D &extent);

		void Cleanup(const VkDevice &device);

		void CleanupFrameBuffers(const VkDevice &device);

		VkSwapchainKHR *GetSwapchin() { return &m_swapchain; }

		uint32_t GetImageCount() { return m_swapchainImageCount; }

		std::vector<VkImageView> GetImageViews() { return m_swapchinImageViews; }

		std::vector<VkFramebuffer> GetFramebuffers() const { return m_framebuffers; }
	};
}