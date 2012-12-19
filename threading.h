/*
 * This is a class that provides a very useful function threadForEach().
 * This will run a function for each object in a vector on as many CPU cores
 * as a computer has. It will return a vector of the return values of the
 * function.
 * 
 * Usage:
 *    R function(T* item) { }
 *    vector<R> results = threadForEach(vector<T> items, function);
 */

#ifndef H_THREADING
#define H_THREADING

#include <thread>
#include <chrono>
#include <vector>
#include <future>
#include <algorithm>
#include <functional>

#include "options.h"

// This mess is to determine CPU cores
#if defined(linux) || defined(__linux) || defined(__linux__) || \
    defined(sun) || defined(__sun) || \
    defined(__APPLE__)
#include <unistd.h>
inline unsigned int core_count() { return sysconf(_SC_NPROCESSORS_ONLN); }
#elif defined(__WIN32) || defined(__WIN32__)
#include <windows.h>
unsigned int core_count() {
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}
//#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
// TODO: figure out how to get # of cores on BSD
#else
// Default to using two threads
inline unsigned int core_count() { return 2; }
#endif

// Manage starting a thread and getting the result
template <class Result, class Type> class Thread
{
	std::thread thr;
	std::future<Result> fut;
	Result return_value;
	Type* item = nullptr;
	bool started  = false;
	bool returned = false;
	std::packaged_task<Result(Type*)> task;

public:
	Thread() { }
	Thread(Result (*function)(Type*), Type* item)
		:task(function) { }

	void start()
	{
		if (!item)
			return;

		started = true;
		fut = task.get_future();
		thr = std::thread(std::move(task), item);
	}

	bool running() const
	{
		// If it's not ready, we'll assume it's still running
		if (started)
			return !(fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
		else
			return false;
	}

	void join()
	{
		if (started)
			thr.join();
	}

	// Get returned result, and allow getting the result multiple times
	Result result()
	{
		if (!started)
			return Result();

		if (returned)
			return return_value;

		returned = true;
		return_value = fut.get();

		return return_value;
	}
};

// Create a generic pool that we will use in our scheduler
template <class Type> class Pool 
{
	typedef typename std::vector<Type>::size_type size_type;

	size_type size;
	std::vector<Type> items;
	std::vector<bool> used;

public:
	Pool(size_type size)
		:size(size), items(size), used(size, false) { }
	
	// Execute on each used item in the pool
	// Usage: Pool.for_each(&Thread::join);
	void for_each(void (Type::*function)())
	{
		std::for_each(items.begin(), items.end(), std::mem_fun_ref(function));
	}

	// Is there an available spot?
	bool available()
	{
		for (size_type i = 0; i < size; ++i)
			if (!used[i])
				return true;

		return false;
	}

	// Get an item from the pool.
	// Note that free() reinitializes the spot in memory.
	Type& get()
	{
		Type* ptr = nullptr;

		for (size_type i = 0; i < size; ++i)
		{
			if (!used[i])
			{
				used[i] = true;
				ptr     = &items[i];
				break;
			}
		}

		if (!ptr)
			throw std::runtime_error("no available spot in pool; always check Pool.avaliable()");

		return *ptr;
	}

	// Allow use of this location in the pool again
	void free(Type& ref)
	{
		bool found = false;

		for (size_type i = 0; i < size; ++i)
		{
			if (&ref == &items[i])
			{
				if (used[i])
					throw std::runtime_error("item in pool already released");

				found    = true;
				used[i]  = false;
				items[i] = Type();
				break;
			}
		}

		if (!found)
			throw std::runtime_error("can't find item in pool to release");
	}
};

// Note that nothing is ever ran unless you call run()
template <class Result, class Type> class ThreadScheduler
{
	typedef typename std::vector<Thread<Result,Type>>::size_type size_type;

	Pool<Thread<Result,Type>> pool;
	std::vector<Thread<Result,Type>> tasks;

public:
	ThreadScheduler(unsigned int size)
		:pool(size) { }
	
	void schedule(Result (*function)(Type*), Type* item)
	{
		tasks.push_back(Thread<Result,Type>(function, item));
	}

	void run()
	{
		// Run all threads
		for (size_type i = 0; i < tasks.size(); ++i)
		{
			while (!pool.available())
				std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_WAIT));
			
			Thread<Result,Type>* t = pool.get();
			t = tasks[i];
			t.start();

			// TODO: this'll die after #CPU threads
		}

		// Wait till they're all done
		pool.for_each(&Thread<Result,Type>::join);
	}
};

// Function object that writes the return value to argument
template<class Result, class Type> class ReturnFunction
{
	Result* ptr;
	Result return_value;
	Result (*function)(Type*);

public:
	ReturnFunction(Result (*function)(Type*), Result* ptr = nullptr)
		:ptr(ptr), function(function) { }

	Result operator()(Type* item)
	{
		return_value = function(item);

		if (ptr)
			*ptr = return_value;

		return return_value;
	}

	Result result() const
	{
		return return_value;
	}
};

// For each CPU core, run function with an item from the vector items
template <class Result, class Type, class Container>
std::vector<Result> threadForEach(Container items, Result (*function)(Type*))
{
	typedef typename std::vector<Type>::size_type size_type;
	typedef typename std::vector<Result>::iterator result_iter;

	std::vector<Result> results(items.size());
	result_iter result = results.begin();
	ThreadScheduler<Result,Type> scheduler(core_count());

	for (Type& item : items)
	{
		scheduler.schedule(ReturnFunction<Result,Type>(function, &*result)(Type*), &item);
		++result;
	}
	
	scheduler.run();
	
	return results;
}

#endif
