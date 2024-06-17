#pragma once
#include "../vkCfg.h"
#include "image.h"

namespace vkImage {
	struct Texture {
		int width;
		int height;
		int channels;
		vk::Image image;
		vk::DeviceMemory imageMemory;
		vk::ImageView imageView;
		vk::Sampler sampler;
		vk::DescriptorSet descriptorSet;
	};

	struct TextureLoadInfo {
		const vk::Device& device;
		const vk::PhysicalDevice& physicalDevice;
		const std::string& filename;
		const vk::CommandPool& commandPool;
		const vk::Queue& queue;
		const vk::DescriptorSetLayout& layout;
		const vk::DescriptorPool& descriptorPool;
	};

	Texture* loadTexture(const TextureLoadInfo* info);

	inline void populateTexture(
		const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::CommandBuffer& commandBuffer,
		const vk::Queue& queue, const Texture* tex, const stbi_uc* pixels
	);

	inline void createSampler(const vk::Device& device, vk::Sampler& sampler);

	inline void createDescriptorSet(const vk::Device& device, const vk::DescriptorPool& descriptorPool, const vk::DescriptorSetLayout& layout, Texture* tex);

	inline void useTexture(const vk::CommandBuffer& commandBuffer, const vk::PipelineLayout& pipelineLayout, const Texture& tex);
}