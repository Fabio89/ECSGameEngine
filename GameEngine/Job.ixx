export module Engine.Job;

import std.core;

export class JobSystem
{
public:
	using Job = std::function<void()>;

	JobSystem(int numThreads) : m_stopFlag(false)
	{
		for (int i = 0; i < numThreads; ++i)
		{
			m_workers.emplace_back([this]
			{
				while (true)
				{
					Job job;
					{
						std::unique_lock<std::mutex> lock{ m_queueMutex };
						m_condition.wait(lock, [this] { return m_stopFlag || !m_jobs.empty(); });
						if (m_stopFlag && m_jobs.empty())
							return;
						job = std::move(m_jobs.front());
						m_jobs.pop();
					}
					job();
				}
			});
		}
	}

	~JobSystem()
	{
		{
			std::unique_lock<std::mutex> lock{ m_queueMutex };
			m_stopFlag = true;
		}
		m_condition.notify_all();
		for (std::thread& worker : m_workers)
		{
			worker.join();
		}
	}

	void enqueueJob(Job job)
	{
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_jobs.push(std::move(job));
		}
		m_condition.notify_one();
	}

private:
	std::vector<std::thread> m_workers;
	std::queue<Job> m_jobs;
	std::mutex m_queueMutex;
	std::condition_variable m_condition;
	std::atomic<bool> m_stopFlag;
};
