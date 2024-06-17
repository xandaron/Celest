#include "image_view.h"
#include "memory.h"
#include "../vkImage/image.h"
#include "../../Game/camera.h"

inline void vkUtil::createImageView(
	const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::Extent2D& extent, const vk::Format& swapchainFormat, ImageView& imageView
) {
	try {
		imageView.imageView = device.createImageView(
			createImageViewCreateInfo(imageView.image, swapchainFormat, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, 1)
		);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to create image view. Reason:\n\t{}", err.what()).c_str());
	}

	createImageViewDepthResources(device, physicalDevice, extent, imageView);
}

inline void vkUtil::createImageViewDepthResources(
	const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::Extent2D& extent, ImageView& imageView
) {
	vkImage::findSupportedFormat(
		physicalDevice,
		{ vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment,
		imageView.depthFormat
	);
	vkImage::makeImage(
		device,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		imageView.depthFormat,
		extent,
		1,
		{},
		imageView.depthBuffer
	);
	vkImage::makeImageMemory(device,
		physicalDevice,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		imageView.depthBuffer,
		imageView.depthMemory
	);
	try {
		imageView.depthView = device.createImageView(
			createImageViewCreateInfo(
				imageView.depthBuffer,
				imageView.depthFormat,
				vk::ImageViewType::e2D,
				vk::ImageAspectFlagBits::eDepth,
				1
			)
		);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to make depth view. Reason:\n\t{}", err.what()).c_str());
	}
}

inline vk::ImageViewCreateInfo vkUtil::createImageViewCreateInfo(
	const vk::Image& image, const vk::Format& format, const vk::ImageViewType& type, const vk::ImageAspectFlagBits& aspect, const uint32_t& layerCount
) {
	/*
	ImageViewCreateInfo(
		vk::ImageViewCreateFlags  flags_			= {},
		vk::Image                 image_			= {},
		vk::ImageViewType         viewType_			= vk::ImageViewType::e1D,
		vk::Format                format_			= vk::Format::eUndefined,
		vk::ComponentMapping	  components_       = {},
		vk::ImageSubresourceRange subresourceRange_ = {}
	) VULKAN_HPP_NOEXCEPT
	*/
	return{
		vk::ImageViewCreateFlagBits(),
		image,
		type,
		format,
		/*
		ComponentMapping(
			vk::ComponentSwizzle r_ = vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle g_ = vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle b_ = vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle a_ = vk::ComponentSwizzle::eIdentity
		) VULKAN_HPP_NOEXCEPT
		*/
		{
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		},
		/*
		ImageSubresourceRange(
			vk::ImageAspectFlags aspectMask_	 = {},
			uint32_t             baseMipLevel_   = {},
			uint32_t             levelCount_     = {},
			uint32_t             baseArrayLayer_ = {},
			uint32_t             layerCount_     = {}
		) VULKAN_HPP_NOEXCEPT
		*/
		{
			aspect,
			0,
			1,
			0,
			layerCount
		}
	};
}

inline void vkUtil::createDescriptorResources(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, ImageView& imageView) {
	/*
	struct BufferInputChunk {
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		size_t size;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags memoryProperties;
	};
	*/
	BufferInputChunk input{
		sizeof(Game::CameraVectors),
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};
	createBuffer(device, physicalDevice, input, imageView.cameraVectorBuffer);

	imageView.cameraVectorWriteLocation = device.mapMemory(imageView.cameraVectorBuffer.bufferMemory, 0, sizeof(Game::CameraVectors));

	input.size = sizeof(Game::CameraMatrices);
	createBuffer(device, physicalDevice, input, imageView.cameraMatrixBuffer);

	imageView.cameraMatrixWriteLocation = device.mapMemory(imageView.cameraMatrixBuffer.bufferMemory, 0, sizeof(Game::CameraMatrices));

	input.size = 1024 * sizeof(glm::mat4);
	input.usage = vk::BufferUsageFlagBits::eStorageBuffer;
	createBuffer(device, physicalDevice, input, imageView.modelBuffer);

	imageView.modelBufferWriteLocation = device.mapMemory(imageView.modelBuffer.bufferMemory, 0, 1024 * sizeof(glm::mat4));

	/*
	imageView.modelTransforms.resize(1024);
	for (int i = 0; i < 1024; ++i) {
		modelTransforms[i] = glm::mat4(1.0);
	}
	*/

	/*
	DescriptorBufferInfo(
		vk::Buffer     buffer_ = {},
        vk::DeviceSize offset_ = {},
        vk::DeviceSize range_  = {}
	) VULKAN_HPP_NOEXCEPT
	*/
	imageView.cameraVectorDescriptor.buffer = imageView.cameraVectorBuffer.buffer;
	imageView.cameraVectorDescriptor.offset = 0;
	imageView.cameraVectorDescriptor.range = sizeof(Game::CameraVectors);

	imageView.cameraMatrixDescriptor.buffer = imageView.cameraMatrixBuffer.buffer;
	imageView.cameraMatrixDescriptor.offset = 0;
	imageView.cameraMatrixDescriptor.range = sizeof(Game::CameraMatrices);

	imageView.ssboDescriptor.buffer = imageView.modelBuffer.buffer;
	imageView.ssboDescriptor.offset = 0;
	imageView.ssboDescriptor.range = 1024 * sizeof(glm::mat4);
}

inline void vkUtil::recordWriteOperations(ImageView& imageView) {
	imageView.writeOps = { 
		createWriteDescriptorSet(imageView.descriptorSet[PipelineType::SKY], vk::DescriptorType::eUniformBuffer, imageView.cameraVectorDescriptor),
		createWriteDescriptorSet(imageView.descriptorSet[PipelineType::STANDARD], vk::DescriptorType::eUniformBuffer, imageView.cameraMatrixDescriptor),
		createWriteDescriptorSet(imageView.descriptorSet[PipelineType::STANDARD], vk::DescriptorType::eStorageBuffer, imageView.ssboDescriptor)
	};
}

inline vk::WriteDescriptorSet vkUtil::createWriteDescriptorSet(
	const vk::DescriptorSet& descriptorSet, const vk::DescriptorType& descriptorType, const vk::DescriptorBufferInfo& descriptorBufferInfo
) {
	/*
	WriteDescriptorSet(
		vk::DescriptorSet               dstSet_           = {},
		uint32_t                        dstBinding_       = {},
		uint32_t                        dstArrayElement_  = {},
		uint32_t                        descriptorCount_  = {},
		vk::DescriptorType              descriptorType_   = vk::DescriptorType::eSampler,
		const vk::DescriptorImageInfo*  pImageInfo_       = {},
		const vk::DescriptorBufferInfo* pBufferInfo_      = {},
		const vk::BufferView*           pTexelBufferView_ = {},
		const void*                     pNext_            = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	return{
		descriptorSet,
		0u,
		0u,
		1u,
		descriptorType,
		{},
		&descriptorBufferInfo,
		{},
		nullptr
	};
}

//void vkUtil::ImageView::makeDepthResources() {
//
//	depthFormat = vkImage::findSupportedFormat(
//		physicalDevice,
//		{ vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint },
//		vk::ImageTiling::eOptimal,
//		vk::FormatFeatureFlagBits::eDepthStencilAttachment
//	);
//
//	vkImage::ImageInputChunk imageInfo;
//	imageInfo.logicalDevice = logicalDevice;
//	imageInfo.physicalDevice = physicalDevice;
//	imageInfo.tiling = vk::ImageTiling::eOptimal;
//	imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
//	imageInfo.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
//	imageInfo.width = width;
//	imageInfo.height = height;
//	imageInfo.format = depthFormat;
//	imageInfo.arrayCount = 1;
//	depthBuffer = vkImage::makeImage(imageInfo);
//	depthBufferMemory = vkImage::makeImageMemory(imageInfo, depthBuffer);
//	depthBufferView = logicalDevice.createImageView(
//		vkInit::createImageViewCreateInfo(
//			depthBuffer, depthFormat, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eDepth, 1
//		)
//	);
//}
//void vkUtil::ImageView::writeDescriptorSet() {
//	logicalDevice.updateDescriptorSets(writeOps, nullptr);
//}