#include "texture.h"
#include "stb_image.h"
#include "../vkUtil/image_view.h"
#include "../vkUtil/memory.h"
#include "../vkInit/descriptors.h"
#include "../vkInit/command_pool.h"

vkImage::Texture* vkImage::loadTexture(const TextureLoadInfo* info) {
	vk::CommandBuffer commandBuffer;
	vkInit::createCommandBuffer(info->device, info->commandPool, commandBuffer);
	Texture* tex = new Texture;
	stbi_uc* pixels = stbi_load(info->filename.c_str(), &tex->width, &tex->height, &tex->channels, STBI_rgb_alpha);
	if (!pixels) {
		Debug::Logger::log(Debug::MINOR_ERROR, std::format("Unable to load texture: {}", info->filename).c_str());
	}
	makeImage(
		info->device, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::Format::eR8G8B8A8Unorm, vk::Extent2D{ tex->width, tex->height }, 1, {}, tex->image
	);
	makeImageMemory(info->device, info->physicalDevice, vk::MemoryPropertyFlagBits::eDeviceLocal, tex->image, tex->imageMemory);
	populateTexture(info->device, info->physicalDevice, commandBuffer, info->queue, tex, pixels);
	free(pixels);
	tex->imageView = info->device.createImageView(
		vkUtil::createImageViewCreateInfo(
			tex->image, vk::Format::eR8G8B8A8Unorm, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, 1
		)
	);
	createSampler(info->device, tex->sampler);
	createDescriptorSet(info->device, info->descriptorPool, info->layout, tex);
	return tex;
}

inline void vkImage::populateTexture(
	const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::CommandBuffer& commandBuffer,
	const vk::Queue& queue, const Texture* tex, const stbi_uc* pixels
) {
	const uint32_t size = tex->width * tex->height * tex->channels;
	Buffer stagingBuffer;
	vkUtil::createBuffer(
		device, physicalDevice, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, stagingBuffer
	);
	
	void* writeLocation = device.mapMemory(stagingBuffer.bufferMemory, 0, size);
	memcpy(writeLocation, pixels, size);
	device.unmapMemory(stagingBuffer.bufferMemory);

	transitionImageLayout(commandBuffer, queue, tex->image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 1);
	copyBufferToImage(commandBuffer, queue, stagingBuffer.buffer, tex->image, { tex->width, tex->height }, 1);
	transitionImageLayout(commandBuffer, queue, tex->image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 1);

	device.freeMemory(stagingBuffer.bufferMemory);
	device.destroyBuffer(stagingBuffer.buffer);
}

inline void vkImage::createSampler(const vk::Device& device, vk::Sampler& sampler) {
	try {
		/*
		SamplerCreateInfo(
			vk::SamplerCreateFlags flags_                   = {},
			vk::Filter             magFilter_               = vk::Filter::eNearest,
			vk::Filter             minFilter_               = vk::Filter::eNearest,
			vk::SamplerMipmapMode  mipmapMode_              = vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode addressModeU_            = vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode addressModeV_            = vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode addressModeW_            = vk::SamplerAddressMode::eRepeat,
			float                  mipLodBias_              = {},
			vk::Bool32             anisotropyEnable_        = {},
			float                  maxAnisotropy_           = {},
			vk::Bool32             compareEnable_           = {},
			vk::CompareOp          compareOp_               = vk::CompareOp::eNever,
			float                  minLod_                  = {},
			float                  maxLod_                  = {},
			vk::BorderColor        borderColor_             = vk::BorderColor::eFloatTransparentBlack,
			vk::Bool32             unnormalizedCoordinates_ = {},
			const void *           pNext_                   = nullptr
		) VULKAN_HPP_NOEXCEPT
		*/
		sampler = device.createSampler({
			vk::SamplerCreateFlags(),
			vk::Filter::eLinear,
			vk::Filter::eNearest,
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat,
			0.0f,
			false,
			1.0f,
			false,
			vk::CompareOp::eAlways,
			0.0f,
			0.0f,
			vk::BorderColor::eIntOpaqueBlack,
			false,
			nullptr
		});
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to make sampler. Reason:\n\t{}", err.what()).c_str());
	}
}

inline void vkImage::createDescriptorSet(const vk::Device& device, const vk::DescriptorPool& descriptorPool, const vk::DescriptorSetLayout& layout, Texture* tex) {
	vkInit::allocateDescriptorSet(device, descriptorPool, layout, tex->descriptorSet);
	/*
	DescriptorImageInfo(
		vk::Sampler     sampler_     = {},
        vk::ImageView   imageView_   = {},
        vk::ImageLayout imageLayout_ = vk::ImageLayout::eUndefined
	) VULKAN_HPP_NOEXCEPT
	*/
	const vk::DescriptorImageInfo imageDescriptor{
		tex->sampler,
		tex->imageView,
		vk::ImageLayout::eShaderReadOnlyOptimal
	};
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
	const vk::WriteDescriptorSet descriptorWrite{
		tex->descriptorSet,
		0,
		0,
		1,
		vk::DescriptorType::eCombinedImageSampler,
		&imageDescriptor,
		nullptr,
		nullptr,
		nullptr
	};
	device.updateDescriptorSets(descriptorWrite, nullptr);
}

inline void vkImage::useTexture(const vk::CommandBuffer& commandBuffer, const vk::PipelineLayout& pipelineLayout, const Texture& tex) {
	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, tex.descriptorSet, nullptr
	);
}
