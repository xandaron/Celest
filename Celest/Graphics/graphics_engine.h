#pragma once 
#include "vkCfg.h"
#include "vkInit/swapchain.h"
#include "vkUtil/image_view.h"
#include "vkImage/texture.h"
#include "vkImage/cubemap.h"
#include "vkMesh/mesh.h"
#include "../../Game/camera.h"
#include "../../Game/scene.h"
#include "../../Game/entity.h"

namespace Graphics {
	

	class Engine {
	public:
		Engine(GLFWwindow* window, Game::Camera* camera);
		
		void loadAssets(const Game::AssetPack& assetPackage);

		void render(Game::Scene* scene);

		~Engine();

	private:
		GLFWwindow* window;

		vk::Instance instance;
		vk::DebugUtilsMessengerEXT debugMessenger;
		vk::DispatchLoaderDynamic dldi;
		vk::SurfaceKHR surface;

		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue graphicsQueue;
		vk::Queue presentQueue;

		Swapchain swapchain;
		std::array<Pipeline, PipelineType::NUMBER_OF_TYPES> pipelines;
		vk::DescriptorPool frameDescriptorPool;
		vk::DescriptorPool meshDescriptorPool;

		vk::CommandPool commandPool;
		vk::CommandBuffer mainCommandBuffer;

		int maxFramesInFlight, frameNumber;

		vkMesh::MeshBuffer meshBuffer;
		std::unordered_map<std::string, vkMesh::Mesh> meshes;
		vkImage::Cubemap cubemap;

		inline void createDebugMessenger();

		inline void createSurface();

		inline void createDevice();

		inline void createImageViews();

		void recreateSwapchain();

		inline void createDescriptorSetLayouts();

		inline void createPipelines();

		inline void createCommandPool();

		inline void createFramebuffers();

		inline void createFrameResources();

		inline void prepareFrame(uint32_t imageIndex, Game::Scene* scene);

		void prepareScene(vk::CommandBuffer commandBuffer);

		inline void recordDrawCommandsSky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Game::Scene* scene);

		inline void recordDrawCommandsScene(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Game::Scene* scene);

		void renderObjects(vk::CommandBuffer commandBuffer, std::string objectType, uint32_t& startInstance, uint32_t instanceCount);

		inline vk::Viewport* createViewport(vk::Extent2D swapchainExtent);

		inline vk::Rect2D* createScissor(vk::Extent2D swapchainExtent);

		void cleanupSwapchain();
	};
}