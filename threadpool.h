#ifndef HWCL3_THREADPOOL_H
#define HWCL3_THREADPOOL_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>

/**
\brief Executes passed functions in threads.
**/
class ThreadPool {
public:

	struct EnqueuedTask
	{
		std::function<void()> handler;
	};

	ThreadPool() : bStop(false)
	{
	}

	void init(size_t numThreads)
	{
		for (size_t i = 0; i < numThreads; ++i)
			threads.emplace_back([this] { // place threads that is waiting for tasks
			for (;;)
			{
				EnqueuedTask enqueuedTask;

				{ // lock tasks
					std::unique_lock<std::mutex> lock(this->queue_mutex); // protect with queue_mutex

																		  // equals to while( !taskAdded or stop ) { wait for condition_var }
					this->condition.wait(lock,
						[this] { return this->bStop || !this->m_enqueuedTasks.empty(); }); // skip accidental unlocks 

					if (this->bStop && this->m_enqueuedTasks.empty())
						return; // quit signaled

					enqueuedTask = std::move(this->m_enqueuedTasks.front());
					this->m_enqueuedTasks.pop();
				} // unlock tasks, task from front is moved to local variable

				enqueuedTask.handler(); // run command handler
			}
		});
	}

	template<class HandlerFunc, class... Args>
	void enqueue(HandlerFunc&& handlerFunc, Args&&... args) // args - to pass object pointer and/or additional parameters
	{
		EnqueuedTask enqueuedTask = {
			std::bind(std::forward<HandlerFunc>(handlerFunc), std::forward<Args>(args)..., std::placeholders::_1)
		};

		{ // lock tasks
			std::unique_lock<std::mutex> lock(queue_mutex); // protect tasks.emplace by queue_mutex

			if( bStop ) throw std::runtime_error("pool was stopped, enqueueing is not possible");

			m_enqueuedTasks.emplace(std::move(enqueuedTask));
		} // unlock tasks

		condition.notify_one(); // notify one thread that is waiting for condition_variable
	}

	/**
	\brief Stops the tasks.
	\details All threads is joined in the destructor.
	**/
	void deinit()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			bStop = true;
		}
		condition.notify_all();
		for (std::thread &worker : threads)
			worker.join();
		threads.clear();
		bStop = false;
	}

	~ThreadPool()
	{
		deinit();
	}

protected:
	std::queue< EnqueuedTask > m_enqueuedTasks;
	std::vector< std::thread > threads;

	std::mutex queue_mutex;
	std::condition_variable condition;
	bool bStop;
};

#endif
