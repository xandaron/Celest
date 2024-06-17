#pragma once
#include "../vkCfg.h"

namespace vkUtil {
	std::vector<char> readFile(const std::string& filename);
	
	vk::ShaderModule createModule(const vk::Device& device, const std::string& filename);
}