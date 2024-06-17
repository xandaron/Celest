#include "queue_family.h"

inline void vkUtil::findQueueFamilies(
	const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, QueueFamilyIndices& queueFamily
) {
	std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
	for (int i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++) {
		/*
		QueueFamilyProperties(
			vk::QueueFlags queueFlags_                  = {},
			uint32_t       queueCount_                  = {},
		*	uint32_t       timestampValidBits_          = {},
		*	vk::Extent3D   minImageTransferGranularity_ = {}
		) VULKAN_HPP_NOEXCEPT
		
		typedef enum VkQueueFlagBits {
			VK_QUEUE_GRAPHICS_BIT       = 0x00000001,
			VK_QUEUE_COMPUTE_BIT        = 0x00000002,
			VK_QUEUE_TRANSFER_BIT       = 0x00000004,
			VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008,
		} VkQueueFlagBits;
		*/
		if ((queueFamilies[i].queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute)) == queueFamilies[i].queueFlags) {
			queueFamily.graphicsFamily = i;
		}
		if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
			queueFamily.presentFamily = i;
		}
		if (queueFamily.isComplete()) {
			break;
		}
	}
}