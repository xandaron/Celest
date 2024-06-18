#pragma once
#include "../vkCfg.h"
#include "../vkUtil/mesh_loader.h"
#include "../vkImage/image.h"
#include "../vkImage/texture.h"

namespace vkJob {

	enum class JobStatus {
		PENDING,
		IN_PROGRESS,
		COMPLETE
	};

	class Job {
	public:
		JobStatus status = JobStatus::PENDING;
		Job* next = nullptr;
		virtual void execute(vk::CommandBuffer commandBuffer, vk::Queue queue) = 0;
	};

	class MakeModel : public Job {
	public:
		vkUtil::MeshLoader* mesh;
		MakeModel(vkUtil::MeshLoader* mesh);
		virtual void execute(vk::CommandBuffer commandBuffer, vk::Queue queue) final;
	};

	class MakeTexture : public Job {
	public:
		vkImage::TextureInputChunk textureInfo;
		vkImage::Texture* texture;
		MakeTexture(vkImage::Texture* texture, vkImage::TextureInputChunk textureInfo);
		virtual void execute(vk::CommandBuffer commandBuffer, vk::Queue queue) final;
	};

	class WorkQueue {
	public:
		Job* first = nullptr, * last = nullptr;
		size_t length = 0;
		std::mutex lock;
		void add(Job* job);
		Job* get_next();
		bool done();
		void clear();
	};
}