#include "memory.h"
#include "single_time_commands.h"

inline void vkUtil::createBuffer(
	const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const size_t& size, const vk::BufferUsageFlags& usage, const vk::MemoryPropertyFlags& memoryProperties, Buffer& buffer
) {
	/*
	BufferCreateInfo(
		vk::BufferCreateFlags flags_                 = {},
		vk::DeviceSize        size_                  = {},
		vk::BufferUsageFlags  usage_                 = {},
		vk::SharingMode       sharingMode_           = vk::SharingMode::eExclusive,
		uint32_t              queueFamilyIndexCount_ = {},
		const uint32_t*       pQueueFamilyIndices_   = {},
		const void*           pNext_                 = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::BufferCreateInfo bufferInfo{
		vk::BufferCreateFlags(),
		size,
		usage,
		vk::SharingMode::eExclusive,
		{},
		{},
		nullptr
	};

	try {
		buffer.buffer = device.createBuffer(bufferInfo);
		allocateBufferMemory(device, physicalDevice, memoryProperties, buffer);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to create buffer. Reason:\n\t{}", err.what()).c_str());
	}
}

inline void vkUtil::allocateBufferMemory(
	const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlags& memoryProperties, Buffer& buffer
) {
	const vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer.buffer);
	/*
	MemoryAllocateInfo(
		vk::DeviceSize allocationSize_  = {},
		uint32_t       memoryTypeIndex_ = {},
		const void*    pNext_           = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	uint32_t index;
	findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, memoryProperties, index);
	vk::MemoryAllocateInfo allocInfo{
		memoryRequirements.size,
		index,
		nullptr
	};

	try {
		buffer.bufferMemory = device.allocateMemory(allocInfo);
		device.bindBufferMemory(buffer.buffer, buffer.bufferMemory, 0);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to allocate buffer memory. Reason:\n\t{}", err.what()).c_str());
	}
}

inline void vkUtil::findMemoryTypeIndex(
	const vk::PhysicalDevice& physicalDevice, const uint32_t& supportedMemoryIndices, const vk::MemoryPropertyFlags& requestedProperties, uint32_t& index
) {
	index = 0;
	vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		const bool supported = static_cast<bool>(supportedMemoryIndices & (1 << i));
		const bool sufficient = (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties;
		if (supported && sufficient) {
			index = i;
			break;
		}
	}
}

inline void vkUtil::copyBuffer(
	const Buffer& srcBuffer, const Buffer& dstBuffer, const vk::DeviceSize& size, const vk::Queue& queue, const vk::CommandBuffer& commandBuffer
) {
	vkUtil::startJob(commandBuffer);
	/*
	BufferCopy(
		vk::DeviceSize srcOffset_ = {},
    	vk::DeviceSize dstOffset_ = {},
    	vk::DeviceSize size_      = {}
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::BufferCopy copyRegion{
		0,
		0,
		size
	};
	commandBuffer.copyBuffer(srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);
	vkUtil::endJob(commandBuffer, queue);
}
