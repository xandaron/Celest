#pragma once
#include "../vkCfg.h"
#include <optional>

namespace vkUtil {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	/**
	* Find suitable queue family indices on the given physical device.
	*
	* @param physicalDevice The physical device to check.
	* @param surface        The present surface.
	* @param queueFamily    Return.
	*/
	inline void findQueueFamilies(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, QueueFamilyIndices& queueFamily);
}