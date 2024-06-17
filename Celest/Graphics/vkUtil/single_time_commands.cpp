#include "single_time_commands.h"

inline void vkUtil::startJob(const vk::CommandBuffer& commandBuffer) {
	commandBuffer.reset();
	/*
	CommandBufferBeginInfo(
		vk::CommandBufferUsageFlags             flags_            = {},
    	const vk::CommandBufferInheritanceInfo* pInheritanceInfo_ = {},
    	const void*                             pNext_            = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::CommandBufferBeginInfo beginInfo{
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
		nullptr,
		nullptr
	};
	commandBuffer.begin(beginInfo);
}

inline void vkUtil::endJob(const vk::CommandBuffer& commandBuffer, const vk::Queue& submissionQueue) {
	commandBuffer.end();
	/*
	SubmitInfo(
		uint32_t                      waitSemaphoreCount_   = {},
    	const vk::Semaphore*          pWaitSemaphores_      = {},
    	const vk::PipelineStageFlags* pWaitDstStageMask_    = {},
    	uint32_t                      commandBufferCount_   = {},
    	const vk::CommandBuffer*      pCommandBuffers_      = {},
    	uint32_t                      signalSemaphoreCount_ = {},
    	const vk::Semaphore*          pSignalSemaphores_    = {},
    	const void*                   pNext_                = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::SubmitInfo submitInfo{
		{},
		{},
		{},
		1,
		&commandBuffer,
		{},
		{},
		nullptr
	};
	submissionQueue.submit(1, &submitInfo, nullptr);
	submissionQueue.waitIdle();
}
