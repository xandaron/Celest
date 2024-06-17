#include "command_pool.h"

inline void vkInit::createCommandPool(
	const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, vk::CommandPool& commandPool
) {
	vkUtil::QueueFamilyIndices queueFamilyIndices;
	vkUtil::findQueueFamilies(physicalDevice, surface, queueFamilyIndices);
	/*
	CommandPoolCreateInfo(
		vk::CommandPoolCreateFlags flags_            = {},
    	uint32_t                   queueFamilyIndex_ = {},
    	const void *               pNext_            = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	const vk::CommandPoolCreateInfo poolInfo{
		vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		queueFamilyIndices.graphicsFamily.value(),
		nullptr
	};

	try {
		Debug::Logger::log(Debug::MESSAGE, "Allocating main command buffer.");
		commandPool = device.createCommandPool(poolInfo);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to create command pool. Reason:\n\t{}", err.what()).c_str());
	}
}

inline void vkInit::createCommandBuffer(const vk::Device& device, const vk::CommandPool& commandPool, vk::CommandBuffer& commandBuffer) {
	/*
	CommandBufferAllocateInfo(
		vk::CommandPool        commandPool_        = {},
    	vk::CommandBufferLevel level_              = vk::CommandBufferLevel::ePrimary,
    	uint32_t               commandBufferCount_ = {},
    	const void *           pNext_              = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::CommandBufferAllocateInfo allocInfo{
		commandPool,
		vk::CommandBufferLevel::ePrimary,
		1,
		nullptr
	};

	try {
		Debug::Logger::log(Debug::MESSAGE, "Allocating main command buffer.");
		commandBuffer = device.allocateCommandBuffers(allocInfo)[0];
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to allocate main command buffer. Reason:\n\t{}", err.what()).c_str());
	}
}

inline void vkInit::createFrameCommandBuffers(const vk::Device& device, const vk::CommandPool& commandPool, std::vector<ImageView>& imageViews) {
	/*
	CommandBufferAllocateInfo(
		vk::CommandPool        commandPool_		   = {},
    	vk::CommandBufferLevel level_			   = vk::CommandBufferLevel::ePrimary,
    	uint32_t               commandBufferCount_ = {},
    	const void *           pNext_              = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::CommandBufferAllocateInfo allocInfo{
		commandPool,
		vk::CommandBufferLevel::ePrimary,
		1,
		nullptr
	};

	for (int i = 0; i < imageViews.size(); ++i) {
		try {
			imageViews[i].commandBuffer = device.allocateCommandBuffers(allocInfo)[0];
			Debug::Logger::log(Debug::MESSAGE, std::format("Allocated command buffer for frame {}.", i));
		}
		catch (vk::SystemError err) {
			throw std::runtime_error(std::format("Failed to allocate command buffer for frame {}. Reason:\n\t{}", i, err.what()).c_str());
		}
	}
}
