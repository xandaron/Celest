#include "swapchain.h"
#include "../vkImage/image.h"
#include "../vkUtil/queue_family.h"
#include "../vkUtil/image_view.h"

inline void vkInit::createSwapchain(
	const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, Swapchain& swapchain
) {
	SwapchainDetails support = querySwapchainSupport(physicalDevice, surface);
	vk::PresentModeKHR presentMode;
	choosePresentMode(support.presentModes, presentMode);
	chooseSurfaceFormat(support.formats, swapchain.format);
	chooseExtent(support.capabilities, swapchain.extent);

	uint32_t imageCount = std::min(
		support.capabilities.maxImageCount,
		support.capabilities.minImageCount + 1
	);
	if (support.capabilities.maxImageCount == 0) {
		imageCount = support.capabilities.minImageCount + 1;
	}
	/*
	VULKAN_HPP_CONSTEXPR SwapchainCreateInfoKHR(
		 vk::SwapchainCreateFlagsKHR	 flags_				    = {},
		 vk::SurfaceKHR					 surface_			    = {},
		 uint32_t						 minImageCount_		    = {},
		 vk::Format						 imageFormat_		    = vk::Format::eUndefined,
		 vk::ColorSpaceKHR				 imageColorSpace_	    = vk::ColorSpaceKHR::eSrgbNonlinear,
		 vk::Extent2D					 imageExtent_		    = {},
		 uint32_t						 imageArrayLayers_	    = {},
		 vk::ImageUsageFlags			 imageUsage_			= {},
		 vk::SharingMode				 imageSharingMode_	    = vk::SharingMode::eExclusive,
		 uint32_t						 queueFamilyIndexCount_ = {},
		 const uint32_t *				 pQueueFamilyIndices_   = {},
		 vk::SurfaceTransformFlagBitsKHR preTransform_		    = vk::SurfaceTransformFlagBitsKHR::eIdentity,
		 vk::CompositeAlphaFlagBitsKHR	 compositeAlpha_	    = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		 vk::PresentModeKHR				 presentMode_		    = vk::PresentModeKHR::eImmediate,
		 vk::Bool32						 clipped_			    = {},
		 vk::SwapchainKHR				 oldSwapchain_		    = {}
	) VULKAN_HPP_NOEXCEPT
	*/
	vk::SwapchainCreateInfoKHR createInfo{
		vk::SwapchainCreateFlagsKHR(),
		surface,
		imageCount,
		swapchain.format.format,
		swapchain.format.colorSpace,
		swapchain.extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		{},
		{},
		support.capabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		presentMode,
		VK_TRUE,
		nullptr
	};

	vkUtil::QueueFamilyIndices queueFamilyIndices;
	vkUtil::findQueueFamilies(physicalDevice, surface, queueFamilyIndices);
	uint32_t queueFamily[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };
	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamily;
	}
	else {
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}
	
	try {
		swapchain.swapchain = device.createSwapchainKHR(createInfo);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to create swapchain. Reason:\n\t{}", err.what()).c_str());
	}

	vkUtil::createImageViews(device, swapchain);
}

inline vkInit::SwapchainDetails vkInit::querySwapchainSupport(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface) {
	return{
		physicalDevice.getSurfaceCapabilitiesKHR(surface),
		physicalDevice.getSurfaceFormatsKHR(surface),
		physicalDevice.getSurfacePresentModesKHR(surface)
	};
}

inline void vkInit::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats, vk::SurfaceFormatKHR& format) {
	format = formats[0];
	for (vk::SurfaceFormatKHR f : formats) {
		if (f.format == vk::Format::eB8G8R8A8Unorm && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			format = f;
			break;
		}
	}
}

inline void vkInit::choosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes, vk::PresentModeKHR& presentMode) {
	presentMode = vk::PresentModeKHR::eFifo;
	for (vk::PresentModeKHR mode : presentModes) {
		if (mode == vk::PresentModeKHR::eMailbox) {
			presentMode = mode;
			break;
		}
	}
}

inline void vkInit::chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, vk::Extent2D& extent) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		extent = capabilities.currentExtent;
	}
	else {
		extent.width = std::min(
			capabilities.maxImageExtent.width,
			std::max(capabilities.minImageExtent.width, extent.width)
		);
		extent.height = std::min(
			capabilities.maxImageExtent.height,
			std::max(capabilities.minImageExtent.height, extent.height)
		);
	}
}
