#pragma once
#include "../vkCfg.h"
#include "../vkUtil/shaders.h"

namespace vkInit {
	struct VertexInputInfo {
		const vk::VertexInputBindingDescription& bindingDescription;
		const std::array<vk::VertexInputAttributeDescription, 3>& attributeDescriptions;
	};

	struct AttachmentInfo {
		const vk::Format& format;
		const uint32_t& index;
	};

	struct shaderInfo {
		const vk::ShaderStageFlagBits& flag;
		const char* shaderPath;
	};

	struct PipelineBuildInfo { 
		const bool& overwrite;
		const VertexInputInfo& vertexInputInfo;
		const std::vector<shaderInfo>& shaderStages;
		const vk::Extent2D& swapchainExtent;
		const AttachmentInfo& colourAttachmentInfo;
		const AttachmentInfo& depthAttachmentInfo;
		const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts;
	};

	inline void buildPipeline(const vk::Device& device, const PipelineBuildInfo& buildInfo, Pipeline& pipeline);

	inline vk::PipelineShaderStageCreateInfo createShaderInfo(const vk::Device& device, const std::string& filename, const vk::ShaderStageFlagBits& stage);

	inline void createShaderStages(
		const vk::Device& device, const std::vector<shaderInfo>& shaderStages, std::vector<vk::PipelineShaderStageCreateInfo>& stages
	);

	inline vk::PipelineVertexInputStateCreateInfo* createVertexInputState(const VertexInputInfo& vertexFormatInfo);

	inline vk::PipelineInputAssemblyStateCreateInfo* createInputAssemblyState();

	inline vk::PipelineViewportStateCreateInfo* createViewportState();

	inline vk::PipelineRasterizationStateCreateInfo* createRasterizationState();

	inline vk::PipelineMultisampleStateCreateInfo* createMultisampleState();

	inline vk::PipelineDepthStencilStateCreateInfo* createDepthStencilState();

	inline vk::PipelineColorBlendAttachmentState* createColourBlendAttachment();

	inline vk::PipelineColorBlendStateCreateInfo* createColourBlendState();

	inline vk::DynamicState* createDynamicState();

	inline vk::PipelineDynamicStateCreateInfo* createDynamicStateInfo();

	inline vk::PipelineLayoutCreateInfo createLayoutInfo(const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);

	inline void createLayout(const vk::Device& device, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, vk::PipelineLayout& layout);

	inline std::vector<vk::AttachmentDescription> createAttachmentDescriptions(
		const bool& overwrite, const vk::Format& depthAttachmentFormat, const vk::Format& colourAttachmentFormat
	);

	inline std::vector<vk::AttachmentReference> createAttachmentReferences(const AttachmentInfo& depthAttachment, const uint32_t& colourAttachmentIndex);

	inline vk::SubpassDescription createSubpass(const std::vector<vk::AttachmentReference>& attachmentReferences);

	inline vk::SubpassDependency* createSubpassDependence();

	inline vk::RenderPassCreateInfo createRenderPassInfo(const vk::SubpassDescription& subpass, const std::vector<vk::AttachmentDescription>& attachmentDescriptions);

	inline void createRenderPass(
		const vk::Device& device, const bool& overwrite, const AttachmentInfo& depthAttachmentInfo, const AttachmentInfo& colourAttachmentInfo, vk::RenderPass& renderPass
	);

	inline void createPipeline(const vk::Device& device, const vk::GraphicsPipelineCreateInfo& pipelineInfo, vk::Pipeline& pipeline);
}