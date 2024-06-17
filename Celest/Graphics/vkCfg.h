#pragma once
#include "../cfg.h"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <unordered_map>

enum PipelineType {
	SKY,
	STANDARD,
	NUMBER_OF_TYPES
};

struct Buffer {
	vk::Buffer buffer;
	vk::DeviceMemory bufferMemory;
};

struct ImageView {
	vk::Image image;
	vk::ImageView imageView;
	vk::Image depthBuffer;
	vk::DeviceMemory depthMemory;
	vk::ImageView depthView;
	vk::Format depthFormat;
	std::array<vk::Framebuffer, PipelineType::NUMBER_OF_TYPES> framebuffer;
	vk::CommandBuffer commandBuffer;
	vk::Semaphore imageAvailable;
	vk::Semaphore renderFinished;
	vk::Fence inFlight;

	Buffer cameraMatrixBuffer;
	void* cameraMatrixWriteLocation;

	Buffer cameraVectorBuffer;
	void* cameraVectorWriteLocation;

	Buffer modelBuffer;
	void* modelBufferWriteLocation;

	vk::DescriptorBufferInfo cameraVectorDescriptor, cameraMatrixDescriptor, ssboDescriptor;
	std::array<vk::DescriptorSet, PipelineType::NUMBER_OF_TYPES> descriptorSet;
	std::vector<vk::WriteDescriptorSet> writeOps;
};

struct Swapchain {
	vk::SwapchainKHR swapchain;
	vk::SurfaceFormatKHR format;
	vk::Extent2D extent;
	std::vector<ImageView> imageViews;
};

struct Pipeline {
	vk::Pipeline pipeline;
	vk::PipelineLayout layout;
	vk::RenderPass renderPass;
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	vk::DescriptorSetLayout frameSetLayout;
	vk::DescriptorSetLayout meshSetLayout;
};
