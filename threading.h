#include <thread>
#include <vector>
#include <future>

using namespace std;

const unsigned int THREAD_WAIT = 50; // Check if we can create new thread every 50 milliseconds

// One of the only system-dependent bits that we need
// TODO: this is not tested on anything except Linux
long core_count()
{
#ifdef _SC_NPROCESSORS_ONLN 
	return sysconf(_SC_NPROCESSORS_ONLN);
#else
#  ifndef __unix__
	return Sys.getenv('NUMBER_OF_PROCESSORS');
#  else 
	return 2;
#  endif
#endif 
}

// There is no pool class in the standard libraries, so this is a simple
// implementation of one that meets the needs of this program
template <class T> class Pool
{
	vector<T> items;
	vector<bool> used;

public:
	Pool() :items(0), used(0, false) { }
	Pool(vector<T>::size_type size)
		:items(size), used(size, false) { }
	
	// Is there an available spot?
	bool available()
	{
		for (vector<T>::size_type i = 0; i < used.size(); ++i)
			if (!in_use)
				return true;

		return false;
	}
	
	// Get an item from the pool.
	// Note that release() reinitializes the spot in memory.
	T& get()
	{
		T* ptr = nullptr;
		
		for (vector<T>::size_type i = 0; i < used.size(); ++i)
		{
			if (!in_use)
			{
				used[i] = true;
				ptr     = &items[i];
				break;
			}
		}

		if (!ptr)
			throw runtime_error("no available spot in pool; always check Pool.avaliable()");

		return *ptr;
	}

	// Allow use of this location in the pool again
	void release(T& ref)
	{
		T* ptr = &ref;
		bool found = false;

		for (vector<T>::size_type i = 0; i < used.size(); ++i)
		{
			if (ptr == &items[i])
			{
				if (used[i] == true)
					throw runtime_error("item in pool already released");

				found   = true;
				used[i] = false;
				items[i] = T();
				break;
			}
		}

		if (!found)
			throw runtime_error("can't find item in pool to release");
	}
};

// Allow easily starting a function in a thread and returning the result
template <class R, class T> class Thread
{
	future<R> fut;
	R return_value;
	bool started  = false;
	bool returned = false;
	packaged_task<R(T&, unsigned int)> task;

public:
	Thread()
	{
	}

	// Initialize everything here so Pool.get().run(...) without the need to
	// reinitialize a custom Thread for each get()
	void run(void (*func)(T&, unsigned int), T& item, unsigned int thread_id)
	{
		started = true;
		task = packaged_task<R(T&, unsigned int)>(func);
		fut  = task.get_future();
		task(item, thread_id);
	}

	// If you really want to...
	void wait()
	{
		if (started)
			fut.wait();
	}

	// Get returned result, and allow getting the result multiple times
	R result()
	{
		if (!started)
			return R();

		if (returned)
			return return_value;

		returned = true;
		return_value = fut.get();

		return return_value;
	}
};

class ThreadPool
{
	Pool<Thread> pool;

public:
	Thread()
		:pool(size)
	{
	}

	Thread* get()
	{

		return pool.get();
	}
};

template <class R, class T, class U> class Threading
{
	T& items;
	ThreadPool pool;
	vector<R> results;
	void (*function)(U&, unsigned int);

public:
	Threading(T& items, void (*func)(U&, unsigned int))
		:items(items), results(items.size()), function(func)
	{
		pool = ThreadPool(core_count());
	}
	
	void run()
	{
		for (U& item : items)
		{
			while (!pool.available())
				this_thread::sleep_for(chrono::milliseconds(THREAD_WAIT));

			Thread t = pool.get();
			results.push_back(t.result());
			t = Thread();
			t.run(function, item, count);
		}
		
		pool.wait();
	}

	const vector<R>& results() const
	{
		return results;
	}
};
