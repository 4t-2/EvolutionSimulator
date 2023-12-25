#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool
{
	public:
		std::thread							**worker = nullptr;
		std::mutex							  jobMutex;
		std::queue<std::function<void(void)>> jobList;
		std::function<void()>				 *job = nullptr;
		std::condition_variable				  isJobAvailable;
		int									  size;
		bool								  terminateSignal = false;
		std::atomic<int>					  count			  = 0;

		void threadLoop(int id)
		{
			while (true)
			{
				job[id] = nullptr;
				{
					std::unique_lock<std::mutex> lock(jobMutex);
					isJobAvailable.wait(lock, [this]() { return !jobList.empty() || terminateSignal; });

					if (terminateSignal)
					{
						return;
					}

					count++;
					job[id] = jobList.front();
					jobList.pop();
				}
				job[id]();
				count--;
			}
		}

		ThreadPool(int size) : size(size)
		{
			worker = new std::thread *[size];
			job	   = new std::function<void()>[size];

			for (int i = 0; i < size; i++)
			{
				worker[i] = new std::thread(&ThreadPool::threadLoop, this, i);
			}

			isJobAvailable.notify_one();
		}

		void queue(std::function<void()> func)
		{
			{
				std::unique_lock<std::mutex> lock(jobMutex);
				jobList.push(func);
			}
			isJobAvailable.notify_one();
		}

		bool active()
		{
			if (count || jobList.size())
			{
				return true;
			}

			return false;
		}

		~ThreadPool()
		{
			{
				std::unique_lock<std::mutex> lock(jobMutex);
				terminateSignal = true;
			}

			isJobAvailable.notify_all();

			for (int i = 0; i < size; i++)
			{
				worker[i]->join();
				delete worker[i];
			}

			delete[] worker;

			delete[] job;
		}
};

// class ThreadDistribute
// {
// 	public:
// 		struct Worker
// 		{
// 				std::thread							 *thread = nullptr;
// 				std::queue<std::function<void(void)>> jobList;
// 				int									  cursor = 0;
// 		} *worker;
//
// 		std::function<void()> *job = nullptr;
// 		int					   size;
// 		bool				   terminateSignal = false;
//
// 		static void threadLoop(Worker &worker, bool &terminateSignal)
// 		{
// 			while (true)
// 			{
// 				while (worker.cursor < worker.jobList.size() && terminateSignal == false)
// 				{
// 				}
//
// 				if (terminateSignal)
// 				{
// 					return;
// 				}
//
// 				worker.jobList.front()();
//
// 				worker.cursor++;
// 			}
// 		}
//
// 		ThreadDistribute(int size) : size(size)
// 		{
// 			worker = new Worker[size];
//
// 			for (int i = 0; i < size; i++)
// 			{
// 				worker[i].thread = new std::thread(threadLoop, worker[i], terminateSignal);
// 			}
// 		}
//
// 		void queue(std::function<void()> func, int id)
// 		{
// 			worker[id].jobList.push(func);
// 		}
//
// 		bool active()
// 		{
// 			for (int i = 0; i < size; i++)
// 			{
// 				if (worker[i].cursor >= worker[i].jobList.size())
// 				{
// 					return true;
// 				}
// 			}
//
// 			return false;
// 		}
//
// 		void clear()
// 		{
// 			for(int i = 0; i < size; i++)
// 			{
// 				worker[i].jobList = std::queue<std::function<void()>>();
// 				worker[i].cursor = 0;
// 			}
// 		}
//
// 		~ThreadDistribute()
// 		{
// 			terminateSignal = true;
//
// 			for (int i = 0; i < size; i++)
// 			{
// 				worker[i].thread->join();
//
// 				delete worker[i].thread;
// 			}
//
// 			delete[] worker;
// 		}
// };
