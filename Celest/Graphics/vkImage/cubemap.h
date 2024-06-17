#pragma once
#include "../vkCfg.h"
#include "Texture.h"

namespace vkImage {
	struct Cubemap {
		bool repeat;
		std::array<vkImage::Texture&, 6> cubefaces;
	};

	inline void createCubemap(const bool& repeated, const std::vector<const Texture*&>& texs, Cubemap& cubemap);
	
	inline void useCubemap(const vk::CommandBuffer& commandBuffer, const vk::PipelineLayout layout, const Cubemap& cubemap);
}
