#include "graphics_engine.h"
#include "vkInit/instance.h"
#include "debug/vkLogging.h"
#include "vkInit/device.h"
#include "vkInit/swapchain.h"
#include "vkInit/descriptors.h"
#include "vkInit/pipeline.h"
#include "vkInit/command_pool.h"
#include "vkInit/sync.h"
#include "vkJob/worker_pool.h"
#include "vkMesh/mesh.h"

Graphics::Engine::Engine(GLFWwindow* window, Game::Camera* camera) {
	this->window = window;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	swapchain.extent = vk::Extent2D{
		width,
		height
	};

	Debug::Logger::log(Debug::MESSAGE, "Creating new graphics engine");
	
	vkInit::makeInstance("Celest", instance);
	createDebugMessenger();
	createSurface();
	createDevice();
	vkInit::createSwapchain(device, physicalDevice, surface, swapchain);
	createImageViews();
	createDescriptorSetLayouts();
	createPipelines();
	createFramebuffers();
	createCommandPool();
	createFrameResources();
}

inline void Graphics::Engine::createDebugMessenger() {
	dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
	Debug::makeDebugMessenger(instance, dldi, debugMessenger);
}

inline void Graphics::Engine::createSurface() {
	VkSurfaceKHR c_style_surface;
	if (glfwCreateWindowSurface(instance, window, nullptr, &c_style_surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a glfw surface for Vulkan.");
	}
	else {
		Debug::Logger::log(Debug::MESSAGE, "Successfully abstracted glfw surface for Vulkan.");
	}
	surface = c_style_surface;
}

inline void Graphics::Engine::createDevice() {
	vkInit::choosePhysicalDevice(instance, surface, physicalDevice);
	vkInit::createLogicalDevice(physicalDevice, surface, device);
	vkUtil::QueueFamilyIndices queueFamily;
	vkUtil::findQueueFamilies(physicalDevice, surface, queueFamily);
	graphicsQueue = device.getQueue(queueFamily.graphicsFamily.value(), 0);
	presentQueue = device.getQueue(queueFamily.presentFamily.value(), 0);
}

inline void Graphics::Engine::createImageViews() {
	const std::vector<vk::Image> images = device.getSwapchainImagesKHR(swapchain.swapchain);
	swapchain.imageViews.resize(images.size());
	for (size_t i = 0; i < images.size(); i++) {
		swapchain.imageViews[i].image = images[i];
		vkUtil::createImageView(device, physicalDevice, swapchain.extent, swapchain.format.format, swapchain.imageViews[i]);
	}
	maxFramesInFlight = static_cast<int>(swapchain.imageViews.size());
	frameNumber = 0;
}

inline void Graphics::Engine::createDescriptorSetLayouts() {
	Debug::Logger::log(Debug::MESSAGE, "Creating descriptor sets.");
	/*
	struct descriptorSetLayoutData {
		int								  count;
		std::vector<uint32_t>			  indices;
		std::vector<vk::DescriptorType>	  types;
		std::vector<uint32_t>			  counts;
		std::vector<vk::ShaderStageFlags> stages;
	};
	*/
	const vkInit::descriptorSetLayoutData frameBindings{
		2,
		{ 0u, 1u },
		{ vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eStorageBuffer },
		{ 1u, 1u },
		{ vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eVertex }
	};
	vkInit::createDescriptorSetLayout(device, frameBindings, pipelines[PipelineType::SKY].frameSetLayout);
	vkInit::createDescriptorSetLayout(device, frameBindings, pipelines[PipelineType::STANDARD].frameSetLayout);

	const vkInit::descriptorSetLayoutData meshBindings{
		1,
		{ 0u },
		{ vk::DescriptorType::eCombinedImageSampler },
		{ 1u },
		{ vk::ShaderStageFlagBits::eFragment }
	};
	vkInit::createDescriptorSetLayout(device, meshBindings, pipelines[PipelineType::SKY].meshSetLayout);
	vkInit::createDescriptorSetLayout(device, meshBindings, pipelines[PipelineType::STANDARD].meshSetLayout);
}

inline void Graphics::Engine::createPipelines() {
	/*
	struct PipelineBuildInfo { 
		const bool overwrite;
		const VertexInputInfo vertexInputInfo;
		const std::vector<shaderInfo> shaderStages;
		const vk::Extent2D swapchainExtent;
		const AttachmentInfo colourAttachmentInfo;
		const AttachmentInfo depthAttachmentInfo;
		const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	};
	*/
	const vkInit::PipelineBuildInfo skyPipelineBuildInfo{
		false,
		{ vkMesh::Vertex::getBindingDescription(), vkMesh::Vertex::getAttributeDescriptions() },
		{
			{ vk::ShaderStageFlagBits::eVertex, "Graphics/shaders/sky_vertex.spv" },
			{ vk::ShaderStageFlagBits::eFragment, "Graphics/shaders/sky_fragment.spv" }
		},
		swapchain.extent,
		{ swapchain.format.format, 0 },
		{ vk::Format::eUndefined, 1},
		{ pipelines[PipelineType::SKY].frameSetLayout, pipelines[PipelineType::SKY].meshSetLayout }
	};
	vkInit::buildPipeline(device, skyPipelineBuildInfo, pipelines[PipelineType::SKY]);

	const vkInit::PipelineBuildInfo pipelineBuildInfo{
		true,
		{ vkMesh::Vertex::getBindingDescription(), vkMesh::Vertex::getAttributeDescriptions() },
		{
			{ vk::ShaderStageFlagBits::eVertex, "Graphics/shaders/vertex.spv" },
			{ vk::ShaderStageFlagBits::eFragment, "Graphics/shaders/fragment.spv" }
			//{ vk::ShaderStageFlagBits::eCompute, "Graphics/shaders/compute.spv" }
		},
		swapchain.extent,
		{ swapchain.format.format, 0 },
		{ swapchain.imageViews[0].depthFormat, 1 },
		{ pipelines[PipelineType::STANDARD].frameSetLayout, pipelines[PipelineType::STANDARD].meshSetLayout }
	};
	vkInit::buildPipeline(device, pipelineBuildInfo, pipelines[PipelineType::STANDARD]);
}

inline void Graphics::Engine::createFramebuffers() {
	for (int i = 0; i < swapchain.imageViews.size(); i++) {
		std::vector<vk::ImageView> attachments{
			swapchain.imageViews[i].imageView
		};
		/*
		FramebufferCreateInfo(
			vk::FramebufferCreateFlags flags_           = {},
			vk::RenderPass             renderPass_      = {},
			uint32_t                   attachmentCount_ = {},
			const vk::ImageView *      pAttachments_    = {},
			uint32_t                   width_           = {},
			uint32_t                   height_          = {},
			uint32_t                   layers_          = {},
			const void *               pNext_           = nullptr
		) VULKAN_HPP_NOEXCEPT
		*/
		const vk::FramebufferCreateInfo skyFramebufferInfo{
			vk::FramebufferCreateFlags(),
			pipelines[PipelineType::SKY].renderPass,
			static_cast<uint32_t>(attachments.size()),
			attachments.data(),
			swapchain.extent.width,
			swapchain.extent.height,
			1,
			nullptr
		};

		try {
			Debug::Logger::log(Debug::MESSAGE, std::format("Creating sky framebuffer for image view {}.", i));
			swapchain.imageViews[i].framebuffer[PipelineType::SKY] = device.createFramebuffer(skyFramebufferInfo);
		}
		catch (vk::SystemError err) {
			Debug::Logger::log(Debug::MINOR_ERROR, std::format("Failed to create framebuffer. Reason\n\t{}", i, err.what()));
		}

		attachments.push_back(swapchain.imageViews[i].depthView);
		const vk::FramebufferCreateInfo standardFramebufferInfo{
			vk::FramebufferCreateFlags(),
			pipelines[PipelineType::STANDARD].renderPass,
			static_cast<uint32_t>(attachments.size()),
			attachments.data(),
			swapchain.extent.width,
			swapchain.extent.height,
			1,
			nullptr
		};

		try {
			Debug::Logger::log(Debug::MESSAGE, std::format("Creating standard framebuffer for image view {}.", i));
			swapchain.imageViews[i].framebuffer[PipelineType::STANDARD] = device.createFramebuffer(standardFramebufferInfo);
		}
		catch (vk::SystemError err) {
			Debug::Logger::log(Debug::MINOR_ERROR, std::format("Failed to create framebuffer. Reason\n\t{}", i, err.what()));
		}
	}
}

inline void Graphics::Engine::createCommandPool() {
	vkInit::createCommandPool(device, physicalDevice, surface, commandPool);
	vkInit::createCommandBuffer(device, commandPool, mainCommandBuffer);
	vkInit::createFrameCommandBuffers(device, commandPool, swapchain.imageViews);
}

inline void Graphics::Engine::createFrameResources() {
	const std::vector<vk::DescriptorType> descriptorSets{
		vk::DescriptorType::eUniformBuffer,
		vk::DescriptorType::eStorageBuffer
	};
	const uint32_t descriptorSetsPerFrame = 2;
	vkInit::createDescriptorPool(device, static_cast<uint32_t>(swapchain.imageViews.size() * descriptorSetsPerFrame), descriptorSets, frameDescriptorPool);
	for (ImageView& imageView : swapchain.imageViews) {
		vkInit::createSemaphore(device, imageView.imageAvailable);
		vkInit::createSemaphore(device, imageView.renderFinished);
		vkInit::createFence(device, imageView.inFlight);
		vkUtil::createDescriptorResources(device, physicalDevice, imageView);
		vkInit::allocateDescriptorSet(device, frameDescriptorPool, pipelines[PipelineType::SKY].frameSetLayout, imageView.descriptorSet[PipelineType::SKY]);
		vkInit::allocateDescriptorSet(device, frameDescriptorPool, pipelines[PipelineType::STANDARD].frameSetLayout, imageView.descriptorSet[PipelineType::STANDARD]);
		vkUtil::recordWriteOperations(imageView);
	}
}

void Graphics::Engine::loadAssets(const Game::AssetPack& assetPack) {
	vkInit::createDescriptorPool(device, 1, { vk::DescriptorType::eCombinedImageSampler }, meshDescriptorPool);

	const uint32_t numAssets = assetPack.objectTypes.size();
	std::vector<const vkImage::Texture*&> textures(numAssets);
	std::vector<const vkImage::Texture*&> cubemapTextures(6);
	std::vector<const vkMesh::MeshData*&> meshesData(numAssets);
	std::queue<vkJob::Job*> jobs;
	for (int i = 0; i < numAssets; i++) {
		/*
		struct TextureLoadInfo {
			const vk::Device& device;
			const vk::PhysicalDevice& physicalDevice;
			const std::string& filename;
			const vk::CommandPool& commandPool;
			const vk::Queue& queue;
			const vk::DescriptorSetLayout& layout;
			const vk::DescriptorPool& descriptorPool;
		};
		*/
		const vkImage::TextureLoadInfo texInfo{
			device,
			physicalDevice,
			assetPack.textureFilenames[i],
			commandPool,
			graphicsQueue,
			pipelines[PipelineType::STANDARD].meshSetLayout,
			meshDescriptorPool
		};
		/*
		struct Job {
			void*       (*fn)(const void*);
			const void* input;
			void*       result;
		}
		*/
		vkJob::Job texJob{
			reinterpret_cast<void* (*)(const void*)>(vkImage::loadTexture),
			&texInfo,
			nullptr
		};
		textures[i] = static_cast<vkImage::Texture*>(texJob.result);
		jobs.push(&texJob);

		const vkMesh::MeshLoadInfo meshInfo{
			assetPack.modelFilenames[i],
			assetPack.preTransforms[i]
		};

		vkJob::Job meshJob{
			reinterpret_cast<void* (*)(const void*)>(vkMesh::loadMesh),
			&meshInfo,
			nullptr
		};
		meshesData[i] = static_cast<vkMesh::MeshData*>(meshJob.result);
		jobs.push(&meshJob);
	}
	for (uint32_t i = 0; i < 6; i++) {
		const vkImage::TextureLoadInfo texInfo{
			device,
			physicalDevice,
			assetPack.skybox[i],
			commandPool,
			graphicsQueue,
			pipelines[PipelineType::STANDARD].meshSetLayout,
			meshDescriptorPool
		};
		vkJob::Job texJob{
			reinterpret_cast<void* (*)(const void*)>(vkImage::loadTexture),
			&texInfo,
			nullptr
		};
		cubemapTextures[i] = static_cast<vkImage::Texture*>(texJob.result);
		jobs.push(&texJob);
		if (assetPack.skyboxRepeatTexture) {
			break;
		}
	}
	vkJob::workerManager(jobs);
	vkMesh::loadVertexBuffer(device, physicalDevice, mainCommandBuffer, graphicsQueue, meshesData, textures, assetPack.objectTypes, meshes, meshBuffer);
	vkImage::createCubemap(assetPack.skyboxRepeatTexture, cubemapTextures, cubemap);
}

inline void Graphics::Engine::prepareFrame(uint32_t imageIndex, Game::Scene* scene) {
	const ImageView& _frame = swapchain.imageViews[imageIndex];
	Game::CameraView cameraViewData = scene->getCamera()->getCameraViewData();
	Game::CameraVectors cameraVectorData{
		{ cameraViewData.forward.x, cameraViewData.forward.y, cameraViewData.forward.z, 0.0 },
		{ cameraViewData.right.x,   cameraViewData.right.y,   cameraViewData.right.z, 0.0 },
		{ cameraViewData.up.x,      cameraViewData.up.y,      cameraViewData.up.z, 0.0 }
	};
	memcpy(_frame.cameraVectorWriteLocation, &(cameraVectorData), sizeof(Game::CameraVectors));

	glm::mat4 view = glm::lookAt(cameraViewData.eye, cameraViewData.center, cameraViewData.up);
	glm::mat4 projection = glm::perspective(glm::radians(45.0), static_cast<double>(swapchain.extent.width) / static_cast<double>(swapchain.extent.height), 0.1, 1000.0);
	projection[1][1] *= -1;

	Game::CameraMatrices cameraMatrixData{
		view,
		projection,
		projection * view
	};
	memcpy(_frame.cameraMatrixWriteLocation, &(cameraMatrixData), sizeof(Game::CameraMatrices));

	size_t i = 0;
	std::vector<glm::mat4> modelTransforms;
	for (std::pair<std::string, std::vector<Entitys::Entity*>> pair : scene->getMappedObjects()) {
		for (Entitys::Entity* obj : pair.second) {
			modelTransforms.push_back(obj->getPhysicsObject()->translationMatrix);
		}
	}
	memcpy(_frame.modelBufferWriteLocation, modelTransforms.data(), i * sizeof(glm::mat4));
	device.updateDescriptorSets(_frame.writeOps, nullptr);
}

inline void Graphics::Engine::prepareScene(vk::CommandBuffer commandBuffer) {
	commandBuffer.setViewport(0, 1, createViewport(swapchain.extent));
	commandBuffer.setScissor(0, 1, createScissor(swapchain.extent));
	commandBuffer.bindVertexBuffers(0, 1, new vk::Buffer{ meshBuffer.vertexBuffer.buffer }, new vk::DeviceSize{0});
	commandBuffer.bindIndexBuffer(meshBuffer.indexBuffer.buffer, 0, vk::IndexType::eUint32);
}

inline void Graphics::Engine::recordDrawCommandsSky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Game::Scene* scene) {
	/*
	RenderPassBeginInfo(
		vk::RenderPass         renderPass_      = {},
		vk::Framebuffer        framebuffer_     = {},
		vk::Rect2D             renderArea_      = {},
		uint32_t               clearValueCount_ = {},
		const vk::ClearValue * pClearValues_    = {},
		const void *           pNext_           = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::RenderPassBeginInfo renderPassInfo{
		pipelines[PipelineType::SKY].renderPass,
		swapchain.imageViews[imageIndex].framebuffer[PipelineType::SKY],
		{ { 0, 0 }, swapchain.extent },
		1u,
		/*
		ClearColorValue(
			const std::array<float, 4>& float32_ = {}
		)
		*/
		new vk::ClearValue{ vk::ClearColorValue({ 0.0f, 0.0f, 0.0f, 1.0f }) },
		nullptr
	};
	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[PipelineType::SKY].pipeline);
	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipelines[PipelineType::SKY].layout,
		0, swapchain.imageViews[imageIndex].descriptorSet[PipelineType::SKY],
		nullptr
	);
	vkImage::useCubemap(commandBuffer, pipelines[PipelineType::SKY].layout, cubemap);
	commandBuffer.draw(6, 1, 0, 0);
	commandBuffer.endRenderPass();
}

inline void Graphics::Engine::recordDrawCommandsScene(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Game::Scene* scene) {
	/**
	* ClearColorValue(
	*	const std::array<float, 4>& float32_ = {}
	* )
	*
	* ClearDepthStencilValue(
	*	float    depth_   = {},
	*	uint32_t stencil_ = {}
	* ) VULKAN_HPP_NOEXCEPT
	*/
	std::vector<vk::ClearValue> clearValues{
		{ vk::ClearColorValue({ 0.0f, 0.0f, 0.0f, 1.0f }) },
		{ vk::ClearDepthStencilValue({ 1.0f, 0 }) }
	};
	/**
	* RenderPassBeginInfo(
	*	vk::RenderPass         renderPass_      = {},
    *	vk::Framebuffer        framebuffer_     = {},
    *	vk::Rect2D             renderArea_      = {},
    *	uint32_t               clearValueCount_ = {},
    *	const vk::ClearValue * pClearValues_    = {},
    *	const void *           pNext_           = nullptr
	* ) VULKAN_HPP_NOEXCEPT
	*/
	vk::RenderPassBeginInfo renderPassInfo{
		pipelines[PipelineType::STANDARD].renderPass,
		swapchain.imageViews[imageIndex].framebuffer[PipelineType::STANDARD],
		{ { 0, 0 }, swapchain.extent },
		static_cast<uint32_t>(clearValues.size()),
		clearValues.data(),
		nullptr
	};

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[PipelineType::STANDARD].pipeline);
	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipelines[PipelineType::STANDARD].layout,
		0, swapchain.imageViews[imageIndex].descriptorSet[PipelineType::STANDARD],
		nullptr
	);

	uint32_t startInstance = 0;
	for (std::pair<std::string, std::vector<Entitys::Entity*>> pair : scene->getMappedObjects()) {
		renderObjects(
			commandBuffer, pair.first, startInstance, static_cast<uint32_t>(pair.second.size())
		);
	}
	commandBuffer.endRenderPass();
}

void Graphics::Engine::renderObjects(vk::CommandBuffer commandBuffer, std::string objectType, uint32_t& startInstance, uint32_t instanceCount) {
	vkMesh::Mesh mesh = meshes[objectType];
	vkImage::useTexture(commandBuffer, pipelines[PipelineType::STANDARD].layout, mesh.texture);
	commandBuffer.drawIndexed(mesh.vertexIndexCount, instanceCount, mesh.firstVertexIndex, 0, startInstance);
	startInstance += instanceCount;
}

inline vk::Viewport* Graphics::Engine::createViewport(vk::Extent2D swapchainExtent) {
	/**
	* Viewport(
	*	float x_ = {},
	*	float y_ = {},
	*	float width_ = {},
	*	float height_ = {},
	*	float minDepth_ = {},
	*	float maxDepth_ = {}
	* ) VULKAN_HPP_NOEXCEPT
	*/
	return new vk::Viewport{
		0.0f,
		0.0f,
		static_cast<float>(swapchainExtent.width),
		static_cast<float>(swapchainExtent.height),
		0.0f,
		1.0f
	};
}

inline vk::Rect2D* Graphics::Engine::createScissor(vk::Extent2D swapchainExtent) {
	/**
	* Rect2D(
	*	vk::Offset2D offset_ = {},
	*	vk::Extent2D extent_ = {}
	* ) VULKAN_HPP_NOEXCEPT
	*/
	return new vk::Rect2D{
		vk::Offset2D{ 0, 0 },
		swapchainExtent
	};
}

void Graphics::Engine::render(Game::Scene* scene) {
	if (device.waitForFences(1, &(swapchain.imageViews[frameNumber].inFlight), VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw std::runtime_error("Fence returned a bad result.");
	}
	
	uint32_t imageIndex;
	try {
		vk::ResultValue acquire = device.acquireNextImageKHR(
			swapchain.swapchain, UINT64_MAX, swapchain.imageViews[frameNumber].imageAvailable, nullptr
		);
		imageIndex = acquire.value;
	}
	catch (vk::OutOfDateKHRError err) {
		recreateSwapchain();
		return;
	}
	catch (vk::IncompatibleDisplayKHRError err) {
		recreateSwapchain();
		return;
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to acquire swapchain image! Reason:\n\t{}", err.what()).c_str());
	}

	if (device.resetFences(1, &(swapchain.imageViews[frameNumber].inFlight)) != vk::Result::eSuccess) {
		throw std::runtime_error("Fence reset returned a bad result.");
	}

	vk::CommandBuffer commandBuffer = swapchain.imageViews[frameNumber].commandBuffer;
	commandBuffer.reset();
	prepareFrame(imageIndex, scene);
	/**
	* CommandBufferBeginInfo(
	*	vk::CommandBufferUsageFlags              flags_            = {},
    *	const vk::CommandBufferInheritanceInfo * pInheritanceInfo_ = {},
    *	const void *                             pNext_            = nullptr
	* ) VULKAN_HPP_NOEXCEPT
	*/
	vk::CommandBufferBeginInfo beginInfo{
		vk::CommandBufferUsageFlags(),
		0,
		nullptr
	};

	try {
		commandBuffer.begin(beginInfo);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to begin recording command buffer! Reason:\n\t{}", err.what()).c_str());
	}

	prepareScene(commandBuffer);
	recordDrawCommandsSky(commandBuffer, imageIndex, scene);
	recordDrawCommandsScene(commandBuffer, imageIndex, scene);

	try {
		commandBuffer.end();
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to record command buffer! Reason:\n\t{}", err.what()).c_str());
	}

	vk::Semaphore waitSemaphores[] = { swapchain.imageViews[frameNumber].imageAvailable };
	vk::Semaphore signalSemaphores[] = { swapchain.imageViews[frameNumber].renderFinished };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	/**
	* SubmitInfo(
	*	uint32_t                      waitSemaphoreCount_   = {},
	*	const vk::Semaphore*          pWaitSemaphores_      = {},
	*	const vk::PipelineStageFlags* pWaitDstStageMask_    = {},
	*	uint32_t                      commandBufferCount_   = {},
	*	const vk::CommandBuffer*      pCommandBuffers_      = {},
	*	uint32_t                      signalSemaphoreCount_ = {},
	*	const vk::Semaphore*          pSignalSemaphores_    = {},
	*	const void*                   pNext_                = nullptr
	* ) VULKAN_HPP_NOEXCEPT
	*/
	vk::SubmitInfo submitInfo = {
		1,
		waitSemaphores,
		waitStages,
		1,
		&commandBuffer,
		1,
		signalSemaphores,
		nullptr
	};

	try {
		graphicsQueue.submit(submitInfo, swapchain.imageViews[frameNumber].inFlight);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to submit draw command buffer! Reason\n\t{}", err.what()).c_str());
	}

	vk::Result result = vk::Result::eSuccess;
	/*
	PresentInfoKHR(
		uint32_t                 waitSemaphoreCount_ = {},
    	const vk::Semaphore *    pWaitSemaphores_    = {},
    	uint32_t                 swapchainCount_     = {},
    	const vk::SwapchainKHR * pSwapchains_        = {},
    	const uint32_t *         pImageIndices_      = {},
    	vk::Result *             pResults_           = {},
    	const void *             pNext_              = nullptr
	) VULKAN_HPP_NOEXCEPT
	*/
	const vk::PresentInfoKHR presentInfo{
		1,
		signalSemaphores,
		1,
		&swapchain.swapchain,
		&imageIndex,
		&result,
		nullptr
	};
	try {
		result = presentQueue.presentKHR(presentInfo);
	}
	catch (vk::SystemError err) {
		if (result != vk::Result::eErrorOutOfDateKHR) {
			throw std::runtime_error(std::format("Failed to present to KHR. Reason\n\t{}", err.what()).c_str());
		}
	}

	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
		recreateSwapchain();
		return;
	}
	frameNumber = (frameNumber + 1) % maxFramesInFlight;
}

void Graphics::Engine::recreateSwapchain() {
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		//glfwWaitEvents();
	}
	device.waitIdle();

	Debug::Logger::log(Debug::DEBUG, "Recreating swapchain.");
	cleanupSwapchain();
	vkInit::createSwapchain(device, physicalDevice, surface, swapchain);
	createImageViews();
	createFramebuffers();
	createFrameResources();
	vkInit::createFrameCommandBuffers(device, commandPool, swapchain.imageViews);
}

void Graphics::Engine::cleanupSwapchain() {
	for (ImageView& imageView : swapchain.imageViews) {
		device.destroyImageView(imageView.imageView);
		device.destroyFramebuffer(imageView.framebuffer[PipelineType::SKY]);
		device.destroyFramebuffer(imageView.framebuffer[PipelineType::STANDARD]);
		device.destroyFence(imageView.inFlight);
		device.destroySemaphore(imageView.imageAvailable);
		device.destroySemaphore(imageView.renderFinished);

		device.unmapMemory(imageView.cameraVectorBuffer.bufferMemory);
		device.freeMemory(imageView.cameraVectorBuffer.bufferMemory);
		device.destroyBuffer(imageView.cameraVectorBuffer.buffer);

		device.unmapMemory(imageView.cameraMatrixBuffer.bufferMemory);
		device.freeMemory(imageView.cameraMatrixBuffer.bufferMemory);
		device.destroyBuffer(imageView.cameraMatrixBuffer.buffer);

		device.unmapMemory(imageView.modelBuffer.bufferMemory);
		device.freeMemory(imageView.modelBuffer.bufferMemory);
		device.destroyBuffer(imageView.modelBuffer.buffer);

		device.destroyImage(imageView.depthBuffer);
		device.freeMemory(imageView.depthMemory);
		device.destroyImageView(imageView.depthView);
	}
	device.destroySwapchainKHR(swapchain.swapchain);
	device.destroyDescriptorPool(frameDescriptorPool);
}

Graphics::Engine::~Engine() {
	device.waitIdle();

	Debug::Logger::log(Debug::MESSAGE, "Destroying graphics engine.");

	device.destroyCommandPool(commandPool);

	for (uint32_t type = 0; type < PipelineType::NUMBER_OF_TYPES; type++) {
		device.destroyPipeline(pipelines[type].pipeline);
		device.destroyPipelineLayout(pipelines[type].layout);
		device.destroyRenderPass(pipelines[type].renderPass);
		device.destroyDescriptorSetLayout(pipelines[type].frameSetLayout);
		device.destroyDescriptorSetLayout(pipelines[type].meshSetLayout);
		for (const vk::PipelineShaderStageCreateInfo& stage : pipelines[type].shaderStages) {
			device.destroyShaderModule(stage.module);
		}
	}

	device.destroyBuffer(meshBuffer.vertexBuffer.buffer);
	device.freeMemory(meshBuffer.vertexBuffer.bufferMemory);
	device.destroyBuffer(meshBuffer.indexBuffer.buffer);
	device.freeMemory(meshBuffer.indexBuffer.bufferMemory);

	cleanupSwapchain();

	device.destroyDescriptorPool(meshDescriptorPool);

	for (const auto& [key, mesh] : meshes) {
		device.freeMemory(mesh.texture.imageMemory);
		device.destroyImage(mesh.texture.image);
		device.destroyImageView(mesh.texture.imageView);
		device.destroySampler(mesh.texture.sampler);
	}

	device.destroy();

	instance.destroySurfaceKHR(surface);
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
	instance.destroy();

	glfwTerminate();
}
