#pragma once
#include "../vkCfg.h"
#include "stb_image.h"

namespace vkImage {
	inline void makeImage(const vk::Device& device, const vk::ImageTiling& tiling, const vk::ImageUsageFlags& usage, const vk::Format& format,
		const vk::Extent2D& extent, const uint32_t& arrayCount, const vk::ImageCreateFlags& flags, vk::Image& image
	);

	inline void makeImageMemory(
		const vk::Device& device, const vk::PhysicalDevice& physicalDevice,
		const vk::MemoryPropertyFlags& memoryProperties, const vk::Image& image, vk::DeviceMemory& imageMemory
	);

	inline void findSupportedFormat(
		const vk::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& candidates,
		const vk::ImageTiling& tiling, const vk::FormatFeatureFlags& features, vk::Format& format
	);

	inline void transitionImageLayout(
		const vk::CommandBuffer& commandBuffer, const vk::Queue& queue, const vk::Image image,
		const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout, const uint32_t& arrayCount
	);

	inline void copyBufferToImage(
		const vk::CommandBuffer& commandBuffer, const vk::Queue& queue, const vk::Buffer& srcBuffer,
		const vk::Image& dstImage, const vk::Extent2D& extent, const uint32_t& arrayCount
	);
}