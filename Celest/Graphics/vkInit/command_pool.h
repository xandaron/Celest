#pragma once
#include "../vkCfg.h"
#include "../vkUtil/queue_family.h"

namespace vkInit {
	inline void createCommandPool(
		const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, vk::CommandPool& commandPool
	);

	inline void createCommandBuffer(const vk::Device& device, const vk::CommandPool& commandPool, vk::CommandBuffer& commandBuffer);

	inline void createFrameCommandBuffers(const vk::Device& device, const vk::CommandPool& commandPool, std::vector<ImageView>& imageViews);
}