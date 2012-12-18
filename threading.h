/*
 * This is a class that provides a very useful function threadForEach().
 * This will run a function for each object in a vector on as many CPU cores
 * as a computer has. It will return a vector of the return values of the
 * function.
 * 
 * Usage:
 *    R function(T item, unsigned int thread_id) { }
 *    vector<R> results = threadForEach(vector<T> items, function);
 */

#ifndef H_THREADING
#define H_THREADING

#include <thread>
#include <chrono>
#include <vector>
#include <future>
// TODO: remove this
#include <iostream>

#include "options.h"

using namespace std;

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
template <class R, class T> class Thread
{
	// Used to start and get results from thread
	thread thr;
	future<R> fut;
	R return_value;
	bool returned = false;
	bool already_started = false;
	packaged_task<R(T*, unsigned int)> task;

	// Passed into function
	T& item;
	unsigned int id;
	
	// Save result when put()
	R& return_val;

public:
	Thread(R (*function)(T*, unsigned int), T& item, unsigned int id, R& return_val)
		:task(function), item(item), id(id), return_val(return_val) { }

	void start()
	{
		already_started = true;
		fut = task.get_future();
		thr = thread(move(task), &item, id);
	}

	bool started() const
	{
		return already_started;
	}

	bool done() const
	{
		if (already_started)
			return (fut.wait_for(chrono::seconds(0)) == future_status::ready);
		else
			return false;
	}

	void wait()
	{
		if (already_started)
			thr.join();
	}

	void put()
	{
		if (already_started)
			return_val = result();
	}

	// Get returned result, and allow getting the result multiple times
	R result()
	{
		if (!already_started)
			return R();

		if (returned)
			return return_value;

		returned = true;
		return_value = fut.get();

		return return_value;
	}
};

// Note that nothing is ever ran unless you call run()
template <class R, class T> class ThreadPool
{
	bool ran = false;
	unsigned int thread_count = 1;
	vector<Thread<R,T>> tasks;

	unsigned int running() const
	{
		unsigned int count = 0;

		for (const Thread<R,T>& t : tasks)
			if (t.started() && !t.done())
				++count;

		return count;
	}

public:
	ThreadPool(unsigned int size) :thread_count(size) { }
	
	void schedule(R (*function)(T*, unsigned int), T& item, unsigned int id, R& return_val)
	{
		tasks.push_back(Thread<R,T>(function, item, id, return_val));
	}

	void run()
	{
		typedef typename vector<Thread<R,T>>::size_type size_type;

		ran = true;

		// Run all threads
		for (size_type i = 0; i < tasks.size(); ++i)
		{
			while (running() > thread_count)
				this_thread::sleep_for(chrono::milliseconds(THREAD_WAIT));

			tasks[i].start();
		}

		// Wait till they're all done
		for (size_type i = 0; i < tasks.size(); ++i)
			tasks[i].wait();

		// Set all the results
		for (size_type i = 0; i < tasks.size(); ++i)
			tasks[i].put();
	}
};

// For each CPU core, run function with an item from the vector items
template <class R, class T>
vector<R> threadForEach(vector<T>& items, R (*function)(T*, unsigned int))
{
	typedef typename vector<T>::size_type size_type;

	ThreadPool<R,T> pool(core_count());
	vector<R> results(items.size());

	for (size_type i = 0; i < items.size(); ++i) 
		pool.schedule(function, items[i], i, results[i]);
	
	pool.run();
	
	return results;
}

#endif
