#pragma once
#include "../vkCfg.h"

namespace vkInit {

	struct SwapchainDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	inline void createSwapchain(
		const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, Swapchain& swapchain
	);

	inline SwapchainDetails querySwapchainSupport(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface);

	inline void chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats, vk::SurfaceFormatKHR& format);

	inline void choosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes, vk::PresentModeKHR& presentMode);

	inline void chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, vk::Extent2D& extent);
}