#pragma once
#include "../vkCfg.h"

namespace vkUtil {
	inline void createImageView(
		const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::Extent2D& extent, const vk::Format& swapchainFormat, ImageView& imageView
	);

	inline void createImageViewDepthResources(
		const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::Extent2D& extent, ImageView& imageView
	);

	inline vk::ImageViewCreateInfo createImageViewCreateInfo(
		const vk::Image& image, const vk::Format& format, const vk::ImageViewType& type, const vk::ImageAspectFlagBits& aspect, const uint32_t& layerCount
	);

	inline void createDescriptorResources(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, ImageView& imageView);

	inline void recordWriteOperations(ImageView& imageView);

	inline vk::WriteDescriptorSet createWriteDescriptorSet(
		const vk::DescriptorSet& descriptorSet, const vk::DescriptorType& descriptorType, const vk::DescriptorBufferInfo& descriptorBufferInfo
	);
}