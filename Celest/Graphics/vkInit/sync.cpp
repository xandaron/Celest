#include "sync.h"

inline void vkInit::createSemaphore(const vk::Device& device, vk::Semaphore& semaphore) {
	/*
	SemaphoreCreateInfo(
		vk::SemaphoreCreateFlags flags_ = {},
		const void*				 pNext_ = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	const vk::SemaphoreCreateInfo semaphoreInfo{
		vk::SemaphoreCreateFlags(),
		nullptr
	};

	try {
		semaphore = device.createSemaphore(semaphoreInfo);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to create semaphore. Reason\n\t{}", err.what()).c_str());
	}
}

inline void vkInit::createFence(const vk::Device& device, vk::Fence& fence) {
	/*
	FenceCreateInfo(
		vk::FenceCreateFlags flags_ = {},
		const void*          pNext_ = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	const vk::FenceCreateInfo fenceInfo{
		vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled,
		nullptr
	};

	try {
		fence = device.createFence(fenceInfo);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to create fence. Reason:\n\t{}", err.what()).c_str());
	}
}
