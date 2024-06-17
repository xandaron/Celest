#pragma once
#include "../vkCfg.h"

namespace vkInit {
	inline void createSemaphore(const vk::Device& device, vk::Semaphore& semaphore);

	inline void createFence(const vk::Device& device, vk::Fence& fence);
}