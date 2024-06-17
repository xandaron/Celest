#pragma once
#include "../vkCfg.h"

namespace vkUtil {
	struct BufferInputChunk {
		size_t size;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags memoryProperties;
	};

	inline void createBuffer(
		const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const size_t& size, const vk::BufferUsageFlags& usage, const vk::MemoryPropertyFlags& memoryProperties, Buffer& buffer
	);

	inline void allocateBufferMemory(
		const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlags& memoryProperties, Buffer& buffer
	);

	inline void findMemoryTypeIndex(
		const vk::PhysicalDevice& physicalDevice, const uint32_t& supportedMemoryIndices, const vk::MemoryPropertyFlags& requestedProperties, uint32_t& index
	);

	inline void copyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, const vk::DeviceSize& size, const vk::Queue& queue, const vk::CommandBuffer& commandBuffer);
}