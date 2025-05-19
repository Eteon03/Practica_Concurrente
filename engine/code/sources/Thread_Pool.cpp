#include<engine/Thread_Pool.hpp>

namespace udit::engine
{
	ThreadPool::ThreadPool(size_t thread_count) : stop(false)
	{
		for (size_t i = 0; i < thread_count; ++i)
		{
			hilos.emplace_back([this]() {this->hilo_loop(); });
		}
	}

	ThreadPool::~ThreadPool()
	{
		shutdown();
	}

	void ThreadPool::shutdown()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}

		condition.notify_all();

		for (std::thread& hilo : hilos)
		{
			if (hilo.joinable())
				hilo.join();
		}
	}

	void ThreadPool::hilo_loop()
	{
		while (true)
		{
			std::function<void()>task;

			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				condition.wait(lock, [this]() {
					return stop || !task_queue.empty();
					});

				if (stop && task_queue.empty())
					return;

				task = std::move(task_queue.front());
				task_queue.pop();
			}
			task();
		}
	}
}