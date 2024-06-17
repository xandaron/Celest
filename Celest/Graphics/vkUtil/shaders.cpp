#include "shaders.h"

std::vector<char> vkUtil::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error(std::format("File \"{}\" not found!", filename).c_str());
	}
	size_t filesize{ static_cast<size_t>(file.tellg()) };

	std::vector<char> buffer(filesize);
	file.seekg(0);
	file.read(buffer.data(), filesize);

	file.close();
	return buffer;
}

vk::ShaderModule vkUtil::createModule(const vk::Device& device, const std::string& filename) {
	const std::vector<char>& sourceCode = readFile(filename);

	vk::ShaderModuleCreateInfo moduleInfo = {};
	moduleInfo.flags = vk::ShaderModuleCreateFlags();
	moduleInfo.codeSize = sourceCode.size();
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

	try {
		return device.createShaderModule(moduleInfo);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error(std::format("Failed to create shader modual for \"{}\". Reason:\n\t{}", filename, err.what()).c_str());
	}
}