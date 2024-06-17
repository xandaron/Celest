#include "descriptors.h"

inline void vkInit::createDescriptorSetLayout(
	const vk::Device& device, const descriptorSetLayoutData& bindings, vk::DescriptorSetLayout& descriptorSetLayout
) {
	std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(bindings.count);
	for (int i = 0; i < bindings.count; i++) {
		/*
		DescriptorSetLayoutBinding(
			uint32_t             binding_			 = {},
			vk::DescriptorType   descriptorType_	 = vk::DescriptorType::eSampler,
			uint32_t             descriptorCount_	 = {},
			vk::ShaderStageFlags stageFlags_		 = {},
			const vk::Sampler *  pImmutableSamplers_ = {}
		) VULKAN_HPP_NOEXCEPT
		*/
		layoutBindings[i] = {
			bindings.indices[i],
			bindings.types[i],
			bindings.counts[i],
			bindings.stages[i],
			{}
		};
	}

	try {
		descriptorSetLayout = device.createDescriptorSetLayout(
			createDescriptorSetLayoutCreateInfo(layoutBindings)
		);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to create Descriptor Set Layout. Reason:\n\t{}", err.what()).c_str());
	}
}

inline vk::DescriptorSetLayoutCreateInfo vkInit::createDescriptorSetLayoutCreateInfo(
	const std::vector<vk::DescriptorSetLayoutBinding>& layoutBindings
) {
	/*
	DescriptorSetLayoutCreateInfo(
		vk::DescriptorSetLayoutCreateFlags    flags_        = {},
		uint32_t                              bindingCount_ = {},
		const vk::DescriptorSetLayoutBinding* pBindings_    = {},
		const void *                          pNext_        = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	return {
		vk::DescriptorSetLayoutCreateFlagBits(),
		static_cast<uint32_t>(layoutBindings.size()),
		layoutBindings.data(),
		nullptr
	};
}

inline void vkInit::createDescriptorPool(
	const vk::Device& device, const uint32_t& size, const std::vector<vk::DescriptorType>& types, vk::DescriptorPool& descriptorPool
) {
	std::vector<vk::DescriptorPoolSize> poolSizes(types.size());
	for (int i = 0; i < static_cast<uint32_t>(types.size()); i++) {
		/*
		DescriptorPoolSize(
			vk::DescriptorType type_            = vk::DescriptorType::eSampler,
        	uint32_t           descriptorCount_ = {}
		) VULKAN_HPP_NOEXCEPT
		*/
		poolSizes[i] = {
			types[i],
			size
		};
	}
	/*
	DescriptorPoolCreateInfo(
		vk::DescriptorPoolCreateFlags  flags_         = {},
    	uint32_t                       maxSets_       = {},
    	uint32_t                       poolSizeCount_ = {},
    	const vk::DescriptorPoolSize * pPoolSizes_    = {},
    	const void *                   pNext_         = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	const vk::DescriptorPoolCreateInfo poolInfo{
		vk::DescriptorPoolCreateFlags(),
		size,
		static_cast<uint32_t>(poolSizes.size()),
		poolSizes.data(),
		nullptr
	};

	try {
		descriptorPool = device.createDescriptorPool(poolInfo);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to create descriptor pool. Reason:\n\t{}", err.what()).c_str());
	}
}

inline void vkInit::allocateDescriptorSet(
	const vk::Device& device, const vk::DescriptorPool& descriptorPool, const vk::DescriptorSetLayout& layout, vk::DescriptorSet& descriptorSet
) {
	/*
	DescriptorSetAllocateInfo(
		vk::DescriptorPool              descriptorPool_     = {},
    	uint32_t                        descriptorSetCount_ = {},
    	const vk::DescriptorSetLayout * pSetLayouts_        = {},
    	const void *                    pNext_              = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	const vk::DescriptorSetAllocateInfo allocationInfo{
		descriptorPool,
		1,
		&layout,
		nullptr
	};

	try {
		descriptorSet = device.allocateDescriptorSets(allocationInfo)[0];
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to allocate descriptor set. Reason:\n\t{}", err.what()).c_str());
	}
}