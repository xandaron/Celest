#pragma once
#include "../vkCfg.h"
#include <thread>
#include <latch>
#include <mutex>
#include <queue>

namespace vkJob {
	struct Job {
		void* (*fn)(const void*);
		const void* input;
		void* result;

		~Job() {
			delete input;
		}
	};

	struct JobQueue {
		std::queue<Job*>& queue;
		std::mutex queueMutex;

		Job* pop() {
			const std::lock_guard<std::mutex> lock(queueMutex);
			if (queue.empty()) {
				return nullptr;
			}
			Job* element = queue.front();
			queue.pop();
			return element;
		}
	};
	
	inline void workerManager(std::queue<Job*>& jobs);

	static void worker(JobQueue& jobQueue, std::latch& finished);
}
