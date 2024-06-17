#include "instance.h"

inline void vkInit::makeInstance(const std::string& applicationName, vk::Instance& instance) {
	const uint32_t version = vk::enumerateInstanceVersion();
	Debug::Logger::log(
		Debug::DEBUG, 
		std::format(
			"System can support vulkan Variant: {}, Major: {}, Minor: {}, Patch: {}",
			VK_API_VERSION_VARIANT(version),
			VK_API_VERSION_MAJOR(version),
			VK_API_VERSION_MINOR(version),
			VK_API_VERSION_PATCH(version)
		)
	);
	
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	extensions.push_back("VK_EXT_debug_utils");

	Debug::Logger::log(Debug::DEBUG, "Required extensions:");
	for (const char* extensionName : extensions) {
		Debug::Logger::log(Debug::DEBUG, std::format("\t\"{}\"", extensionName));
	}
	std::vector<const char*> layers{ "VK_LAYER_KHRONOS_validation" };
	checkSupport(extensions, layers);
	/*
	VULKAN_HPP_CONSTEXPR ApplicationInfo(
		const char * pApplicationName_   = {},
		uint32_t     applicationVersion_ = {},
		const char * pEngineName_        = {},
		uint32_t     engineVersion_      = {},
		uint32_t     apiVersion_         = {}
	)
	*/
	const vk::ApplicationInfo appInfo = {
		applicationName.c_str(),
		version,
		"Celest Graphics Engine",
		version,
		version
	};
	/*
	InstanceCreateInfo(
		VULKAN_HPP_NAMESPACE::InstanceCreateFlags     flags_                   = {},
		const VULKAN_HPP_NAMESPACE::ApplicationInfo * pApplicationInfo_        = {},
		uint32_t                                      enabledLayerCount_       = {},
		const char * const *                          ppEnabledLayerNames_     = {},
		uint32_t                                      enabledExtensionCount_   = {},
		const char * const *						  ppEnabledExtensionNames_ = {}
	)
	*/
	const vk::InstanceCreateInfo createInfo = {
		vk::InstanceCreateFlags(),
		&appInfo,
		static_cast<uint32_t>(layers.size()),
		layers.data(),
		static_cast<uint32_t>(extensions.size()),
		extensions.data()
	};

	try {
		instance = vk::createInstance(createInfo);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Vulkan failed to create instance. Reason:\n\t{}", err.what()).c_str());
	}
}

inline void vkInit::checkSupport(const std::vector<const char*>& extensions, const std::vector<const char*>& layers) {
	const std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();
	bool found;
	for (const char* extension : extensions) {
		found = false;
		for (vk::ExtensionProperties supportedExtension : supportedExtensions) {
			if (strcmp(extension, supportedExtension.extensionName) == 0) {
				found = true;
				Debug::Logger::log(Debug::MESSAGE, std::format("Extension \"{}\" is supported!", extension));
				break;
			}
		}
		if (!found) {
			throw std::runtime_error(std::format("Required format \"{}\" is not supported.", extension).c_str());
		}
	}

	const std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();
	for (const char* layer : layers) {
		found = false;
		for (vk::LayerProperties supportedLayer : supportedLayers) {
			if (strcmp(layer, supportedLayer.layerName) == 0) {
				found = true;
				Debug::Logger::log(Debug::MESSAGE, std::format("Layer \"{}\" is supported!", layer));
				break;
			}
		}
		if (!found) {
			throw std::runtime_error(std::format("Required layer \"{}\" is not supported.", layer).c_str());
		}
	}
}