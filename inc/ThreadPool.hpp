#include <atomic>
#include <condition_variable>
#include <functional>
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
			if(count || jobList.size())
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
