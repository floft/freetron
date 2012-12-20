/*
 * This is a class that provides a very useful function threadForEach().
 * This will run a function for each object in a container on as many CPU cores
 * as a computer has. It will return a vector of the return values of running
 * the function.
 * 
 * Usage:
 *    R function(T* item) { }
 *    vector<R> results = threadForEach(container<T> items, function);
 *
 * Notes:
 *    - container can be any container that has .begin(), .end(), and .size()
 *    - This *should* create as many threads as your CPU has cores
 */

#ifndef H_THREADING
#define H_THREADING

#include <thread>
#include <chrono>
#include <vector>
#include <future>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "cores.h"
#include "options.h"

// Manage starting a thread and getting the result
template <class Result, class Type> class Thread
{
	bool started  = false;
	bool returned = false;
	Result* ret = nullptr;
	std::thread thread;
	std::future<Result> future;
	std::packaged_task<Result(Type*)> task;

public:
	Thread() { }

	void start(Result (*function)(Type*), Type* item, Result* return_location)
	{
		started  = true;
		returned = false;
		ret = return_location;
		task = std::packaged_task<Result(Type*)>(function);	
		future = task.get_future();
		thread = std::thread(std::move(task), item);
	}

	bool running() const
	{
		// If it's not ready, we'll assume it's still running
		if (started)
			return !(future.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
		else
			return false;
	}

	// Save the result of the function executed in this thread
	void put()
	{
		if (started && !returned)
		{
			// For some reason future.get() doesn't necessarily block
			thread.join();

			returned = true;
			*ret = future.get();
		}
	}

	// Save result before destroying
	~Thread()
	{
		put();
	}
};

// Note that nothing is ever ran unless you call run()
template <class Result, class Type> class ThreadScheduler
{
	std::vector<Thread<Result,Type>> pool;

public:
	ThreadScheduler(unsigned int size)
		:pool(size) { }
	
	void schedule(Result (*function)(Type*), Type* item, Result* return_location)
	{
		// Get or wait to get one of the threads
		Thread<Result,Type>* thread;

		while (!(thread = get()))
			std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_WAIT));

		// Save the output of the previous function running on this thread
		thread->put();
		// Start running our new stuff
		thread->start(function, item, return_location);
	}

	void wait()
	{
		for (Thread<Result,Type>& t : pool)
			t.put();
	}

	~ThreadScheduler()
	{
		wait();
	}

private:
	Thread<Result,Type>* get()
	{
		for (Thread<Result,Type>& t : pool)
			if (!t.running())
				return &t;

		return nullptr;
	}
};

// For each CPU core, run function with an item from the vector items
template <class Result, class Type, class Container>
std::vector<Result> threadForEach(Container items, Result (*function)(Type*))
{
	typedef typename std::vector<Result>::iterator result_iter;

	std::vector<Result> results(items.size());
	result_iter result = results.begin();
	ThreadScheduler<Result,Type> scheduler(core_count());

	for (Type& item : items)
	{
		scheduler.schedule(function, &item, &*result);
		++result;
	}
	
	scheduler.wait();
	
	return results;
}

#endif
