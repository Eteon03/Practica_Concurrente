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
		//Crea una pool de hilos siendo su cantidad tantos como nucleos haya disponibles.
		explicit ThreadPool(size_t thread_count = std::thread::hardware_concurrency());
		
		//Destructor de pool
		~ThreadPool();

		//Se evita la copia del ThreadPool debido a mutex
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		//Se atribuye una tarea a la cola, y se devuelve un future asociado a su resultado
		template<typename Func, typename... Args>
		auto submit(Func&& f, Args&&... args) -> std::future<decltype(f(args...))>
		{
			using return_type = decltype (f(args...));

			//Se empaqueta la tarea
			auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));

			//Se obtiene el future del resultado
			std::future<return_type> res = task->get_future();

			{
				std::unique_lock<std::mutex>lock(queue_mutex); //Se protege la cola de tareas
				task_queue.emplace([task]() {(*task)(); });//Se añade la tarea a la cola
			}

			condition.notify_one(); //Se notifica a los hilos de la disponibilidad de una tarea
			return res;// Se devuelve el future para esperar el resultado
		}
		//Metodo para detener los hilos y vaciar la cola de tareas
		void shutdown();

	private:
		std::vector<std::thread>hilos;					//Contenedor de hilos de trabajo
		std::queue<std::function<void()>> task_queue;	//Cola de tareas
		std::mutex queue_mutex;							//Mutex para proteger el acceso a la cola
		std::condition_variable condition;				//Condicion para notificar a los hilos
		std::atomic<bool> stop;							//Flag que indica al pool que debe detenerse

		//Espera tareas y ejecuta
		void hilo_loop();

	};
}