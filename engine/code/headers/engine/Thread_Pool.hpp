#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <future>
#include <functional>
#include <condition_variable>
#include <atomic>

namespace udit::engine
{
	class ThreadPool
	{
	public:
		explicit ThreadPool(size_t thread_count = std::thread::hardware_concurrency());
		~ThreadPool();

		//Se prohibe la copia del ThreadPool
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		//Se atribuye una tarea a la cola, y se devuelve un future asociado a su resultado
		template<typename Func, typename... Args>
		auto submit(Func&& f, Args&&... args) -> std::future<decltype(f(args...))>
		{
			using return_type = decltype (f(args...));
			auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));

			std::future<return_type> res = task->get_future();

			{
				std::unique_lock<std::mutex>lock(queue_mutex);
				task_queue.emplace([task]() {(*task)(); });
			}

			condition.notify_one();
			return res;
		}
		void shutdown();

	private:
		std::vector<std::thread>hilos;
		std::queue<std::function<void()>> task_queue;
		std::mutex queue_mutex;
		std::condition_variable condition;
		std::atomic<bool> stop;

		void hilo_loop();

	};
}