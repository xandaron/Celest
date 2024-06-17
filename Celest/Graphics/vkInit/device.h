#pragma once
#include "../vkCfg.h"
#include "../vkUtil/queue_family.h"

namespace vkInit {
	/**
	* Choose a physical device for the vulkan instance.
	*
	* @param instance       The vulkan instance to use.
	* @param physicalDevice The chosen physical device.
	*
	* @throws std::runtime_error No usable device found.
	*/
	inline void choosePhysicalDevice(const vk::Instance& instance, const vk::SurfaceKHR& surface, vk::PhysicalDevice& physicalDevice);

	/**
	* Check whether the given physical device is suitable for use.
	*
	* @param device  The physical device.
	* @param surface The present surface.
	* @param score   Suitability score of the device.
	*/
	inline void rateDeviceSuitability(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface, uint32_t& score);

	/**
	* Check whether the physical device can support the given extensions.
	*
	* @param device			     The physical device to check.
	* @param requestedExtensions A list of extension names to check against.
	*
	* @return Whether all of the requested extensions are supported by the device.
	*/
	bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device, std::vector<const char*>& requestedExtensions);
	
	/**
	* Create a Vulkan device.
	*
	* @param physicalDevice The Physical Device to represent.
	* @param surface		The window surface.
	* @param device			The created device.
	*
	* @throws std::runtime_error If a logical device could not be created.
	*/
	inline void createLogicalDevice(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, vk::Device& device);
}