#pragma once
#include "../vkCfg.h"

namespace vkInit {
	struct descriptorSetLayoutData {
		const int& count;
		const std::vector<uint32_t>& indices;
		const std::vector<vk::DescriptorType>& types;
		const std::vector<uint32_t>& counts;
		const std::vector<vk::ShaderStageFlags>& stages;
	};

	inline void createDescriptorSetLayout(
		const vk::Device& device, const descriptorSetLayoutData& bindings, vk::DescriptorSetLayout& descriptorSetLayout
	);
	
	inline vk::DescriptorSetLayoutCreateInfo createDescriptorSetLayoutCreateInfo(
		const std::vector<vk::DescriptorSetLayoutBinding>& layoutBindings
	);

	inline void createDescriptorPool(
		const vk::Device& device, const uint32_t& size, const std::vector<vk::DescriptorType>& types, vk::DescriptorPool& descriptorPool
	);

	inline void allocateDescriptorSet(
		const vk::Device& device, const vk::DescriptorPool& descriptorPool, const vk::DescriptorSetLayout& layout, vk::DescriptorSet& descriptorSet
	);
}