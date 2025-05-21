#include<engine/Thread_Pool.hpp>

namespace udit::engine
{
	//Crea hilos y los pone en espera dentro del loop
	ThreadPool::ThreadPool(size_t thread_count) : stop(false)
	{
		for (size_t i = 0; i < thread_count; ++i)
		{
			//Se ejecuta la funcion por cada hilo de forma indefinida
			hilos.emplace_back([this]() {this->hilo_loop(); });
		}
	}

	ThreadPool::~ThreadPool()
	{
		shutdown();
	}

	//Notifica a los hilos para que se detengan
	void ThreadPool::shutdown()
	{
		{
			//Se usa mutex para modificar la bandera de parada
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}

		condition.notify_all();

		//Se espera a que los hilos finalizen su ejecucion
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
				//Espera a que haya una tarea disponible
				std::unique_lock<std::mutex> lock(queue_mutex);
				condition.wait(lock, [this]() {
					return stop || !task_queue.empty();
					});

				//Si se detiene y no hay tareas disponibles sale del hilo
				if (stop && task_queue.empty())
					return;

				//Obtenemos la siguiente tarea
				task = std::move(task_queue.front());
				task_queue.pop();
			}
			//Ejecutamos la tarea fuera del lock mutex
			task();
		}
	}
}