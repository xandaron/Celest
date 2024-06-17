#pragma once
#include "../vkCfg.h"
namespace vkUtil {
	inline void startJob(const vk::CommandBuffer& commandBuffer);

	inline void endJob(const vk::CommandBuffer& commandBuffer, const vk::Queue& submissionQueue);
}