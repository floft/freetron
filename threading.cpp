#include <thread>
#include <unistd.h>
#include <iostream>

using namespace std;

template <class T>
class Threading
{
	long cpus;
	thread* threads;
	void (*function)(T);

public:
	Threading(void (*func)(T))
		:function(func)
	{
		cpus = sysconf(_SC_NPROCESSORS_ONLN);
		threads = new thread[cpus];
	}
	
	void start()
	{
		for (int i = 0; i < cpus; ++i)
			threads[i] = thread(function, i);
			//threads[i] = thread([i,this] { function(i); });
	}

	void wait()
	{
		for (int i = 0; i < cpus; ++i)
			threads[i].join();
	}

	~Threading() { delete[] threads; }
};

void print(int thread)
{
	cout << thread;
}

void loop(int thread)
{
	int q = 2;
	int big = 1000;

	for (int a = 0; a < big; ++a)
	for (int b = 0; b < big; ++b)
	for (int c = 0; c < big; ++c)
		q*=10;

	cout << thread;
}

int main()
{
	//Threading<int> t(print);
	Threading<int> t(loop);
	t.start();
	cout << "M";
	t.wait();
	cout << endl;

	return 0;
}
