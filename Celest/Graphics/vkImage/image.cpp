#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../vkUtil/memory.h"
#include "../control/logging.h"
#include "../vkUtil/single_time_commands.h"
#include "../vkInit/descriptors.h"

inline void vkImage::makeImage(const vk::Device& device, const vk::ImageTiling& tiling, const vk::ImageUsageFlags& usage, const vk::Format& format,
	const vk::Extent2D& extent, const uint32_t& arrayCount, const vk::ImageCreateFlags& flags, vk::Image& image
) {
	try {
		/*
		ImageCreateInfo(
			vk::ImageCreateFlags    flags_                 = {},
			vk::ImageType           imageType_             = vk::ImageType::e1D,
			vk::Format              format_                = vk::Format::eUndefined,
			vk::Extent3D            extent_                = {},
			uint32_t                mipLevels_             = {},
			uint32_t                arrayLayers_           = {},
			vk::SampleCountFlagBits samples_               = vk::SampleCountFlagBits::e1,
			vk::ImageTiling         tiling_                = vk::ImageTiling::eOptimal,
			vk::ImageUsageFlags     usage_                 = {},
			vk::SharingMode         sharingMode_           = vk::SharingMode::eExclusive,
			uint32_t                queueFamilyIndexCount_ = {},
			const uint32_t *        pQueueFamilyIndices_   = {},
			vk::ImageLayout         initialLayout_         = vk::ImageLayout::eUndefined,
			const void *            pNext_                 = nullptr
		) VULKAN_HPP_NOEXCEPT
		*/
		image = device.createImage({
			vk::ImageCreateFlagBits() | flags,
			vk::ImageType::e2D,
			format,
			vk::Extent3D(extent, 1),
			1,
			arrayCount,
			vk::SampleCountFlagBits::e1,
			tiling,
			usage,
			vk::SharingMode::eExclusive,
			{},
			{},
			vk::ImageLayout::eUndefined,
			nullptr
		});
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Unable to create image. Reason:\n\t{}", err.what()));
	}
}

inline void vkImage::makeImageMemory(
	const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlags& memoryProperties,
	const vk::Image& image, vk::DeviceMemory& imageMemory
) {
	const vk::MemoryRequirements requirements = device.getImageMemoryRequirements(image);
	uint32_t index;
	vkUtil::findMemoryTypeIndex(physicalDevice, requirements.memoryTypeBits, memoryProperties, index);
	/*
	MemoryAllocateInfo(
		vk::DeviceSize allocationSize_  = {},
		uint32_t       memoryTypeIndex_ = {},
		const void *   pNext_           = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	const vk::MemoryAllocateInfo allocation{
		requirements.size,
		index,
		nullptr
	};

	try {
		imageMemory = device.allocateMemory(allocation);
		device.bindImageMemory(image, imageMemory, 0);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Unable to allocate memory for image. Reason\n\t{}", err.what()).c_str());
	}
}

inline void vkImage::findSupportedFormat(
	const vk::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& candidates, const vk::ImageTiling& tiling,
	const vk::FormatFeatureFlags& features, vk::Format& format
) {
	bool found = false;
	for (vk::Format candidate : candidates) {
		/*
		FormatProperties(
			vk::FormatFeatureFlags linearTilingFeatures_  = {},
			vk::FormatFeatureFlags optimalTilingFeatures_ = {},
			vk::FormatFeatureFlags bufferFeatures_        = {}
		) VULKAN_HPP_NOEXCEPT
		*/
		vk::FormatProperties properties = physicalDevice.getFormatProperties(candidate);
		if (tiling == vk::ImageTiling::eLinear
			&& (properties.linearTilingFeatures & features) == features
			) {
			format = candidate;
			found = true;
			break;
		}
		if (tiling == vk::ImageTiling::eOptimal
			&& (properties.optimalTilingFeatures & features) == features
			) {
			format = candidate;
			found = true;
			break;
		}
	}
	if (!found) {
		throw std::runtime_error("Unable to find suitable format.");
	}
}

inline void vkImage::transitionImageLayout(
	const vk::CommandBuffer& commandBuffer, const vk::Queue& queue, const vk::Image image,
	const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout, const uint32_t& arrayCount
) {
	vkUtil::startJob(commandBuffer);
	/*
	typedef struct VkImageSubresourceRange {
		VkImageAspectFlags    aspectMask;
		uint32_t              baseMipLevel;
		uint32_t              levelCount;
		uint32_t              baseArrayLayer;
		uint32_t              layerCount;
	} VkImageSubresourceRange;
	*/
	const vk::ImageSubresourceRange access{
		vk::ImageAspectFlagBits::eColor,
		0,
		1,
		0,
		arrayCount
	};
	/*
	ImageMemoryBarrier(
		vk::AccessFlags           srcAccessMask_       = {},
        vk::AccessFlags           dstAccessMask_       = {},
        vk::ImageLayout           oldLayout_           = vk::ImageLayout::eUndefined,
        vk::ImageLayout           newLayout_           = vk::ImageLayout::eUndefined,
        uint32_t                  srcQueueFamilyIndex_ = {},
        uint32_t                  dstQueueFamilyIndex_ = {},
        vk::Image                 image_               = {},
        vk::ImageSubresourceRange subresourceRange_    = {},
        const void *              pNext_               = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::ImageMemoryBarrier barrier{
		{},
		{},
		oldLayout,
		newLayout,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		image,
		access,
		nullptr
	};
	vk::PipelineStageFlags sourceStage, destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined
		&& newLayout == vk::ImageLayout::eTransferDstOptimal) {

		barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else {

		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);
	vkUtil::endJob(commandBuffer, queue);
}

inline void vkImage::copyBufferToImage(
	const vk::CommandBuffer& commandBuffer, const vk::Queue& queue, const vk::Buffer& srcBuffer,
	const vk::Image& dstImage, const vk::Extent2D& extent, const uint32_t& arrayCount
) {
	vkUtil::startJob(commandBuffer);
	/*
	ImageSubresourceLayers(
		vk::ImageAspectFlags aspectMask_     = {},
        uint32_t             mipLevel_       = {},
        uint32_t             baseArrayLayer_ = {},
        uint32_t             layerCount_     = {}
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::ImageSubresourceLayers access{
		vk::ImageAspectFlagBits::eColor,
		0,
		0,
		arrayCount
	};
	/*
	BufferImageCopy(
		vk::DeviceSize             bufferOffset_      = {},
        uint32_t                   bufferRowLength_   = {},
        uint32_t                   bufferImageHeight_ = {},
        vk::ImageSubresourceLayers imageSubresource_  = {},
        vk::Offset3D               imageOffset_       = {},
        vk::Extent3D               imageExtent_       = {}
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::BufferImageCopy copy{
		0,
		0,
		0,
		access,
		vk::Offset3D(0, 0, 0),
		vk::Extent3D(extent, 1)
	};
	copy.imageSubresource = access;
	commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, copy);
	vkUtil::endJob(commandBuffer, queue);
}
