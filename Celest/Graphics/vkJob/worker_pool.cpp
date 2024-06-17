#include "worker_pool.h"

inline void vkJob::workerManager(std::queue<Job*>& jobs) {
	const uint32_t threadCount = std::min(std::thread::hardware_concurrency() - 1, static_cast<uint32_t>(jobs.size()));
	JobQueue jobQueue{ jobs };
	std::latch workFinished{ threadCount };
	Debug::Logger::log(Debug::DEBUG, std::format("Thread count: {}, Job count: {}", threadCount, jobs.size()));
	for (uint32_t i = 0; i < threadCount; i++) {
		std::thread(worker, jobQueue, workFinished).detach();
	}
	workFinished.wait();
	Debug::Logger::log(Debug::DEBUG, "Thread work finished.");
}

static void vkJob::worker(JobQueue& jobQueue, std::latch& finished) {
	for (Job* job = jobQueue.pop(); job; job = jobQueue.pop()) {
		try {
			job->result = job->fn(job->input);
			delete job;
			Debug::Logger::log(Debug::DEBUG, "Job complete.");
		}
		catch (std::exception err) {
			Debug::Logger::log(Debug::MINOR_ERROR, std::format("Thread job failed. Reason:\n\t{}", err.what()));
		}
	}
	finished.count_down();
}
