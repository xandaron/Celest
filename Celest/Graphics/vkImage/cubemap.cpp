#include "cubemap.h"

inline void vkImage::createCubemap(const bool& repeated, const std::vector<const Texture*&>& texs, Cubemap& cubemap) {
	std::array<Texture&, 6> cubeTex;
	if (repeated) {
		cubeTex[0] = *texs[0];
		cubemap = {
			true,
			cubeTex
		};
		return;
	}
	for (uint32_t i = 0; i < 6; i++) {
		cubeTex[i] = *texs[i];
	}
	cubemap = {
		false,
		cubeTex
	};
}

inline void vkImage::useCubemap(const vk::CommandBuffer& commandBuffer, const vk::PipelineLayout layout, const Cubemap& cubemap) {
	if (cubemap.repeat) {
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 1, cubemap.cubefaces[0].descriptorSet, nullptr);
		return;
	}
	for (const Texture& tex : cubemap.cubefaces) {
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 1, tex.descriptorSet, nullptr);
	}
}