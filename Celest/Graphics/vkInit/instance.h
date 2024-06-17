#pragma once
#include "../vkCfg.h"

namespace vkInit {
	/**
	* Create a Vulkan instance.
	*
	* @param applicationName The name of the application.
	* @param instance		 A ref to the instance variable.
	* 
	* @throws std::runtime_error Instance couldn't be created.
	*/
	inline void makeInstance(const std::string& applicationName, vk::Instance& instance);

	/**
	* Check whether the requested extensions and layers are supported.
	*
	* @param extensions A list of extension names being requested.
	* @param layers		A list of layer names being requested.
	*
	* @throws std::runtime_error Extension or layer isn't supported.
	*/
	inline void checkSupport(const std::vector<const char*>& extensions, const std::vector<const char*>& layers);
}