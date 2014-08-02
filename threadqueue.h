/*
 * Function returns a result version.
 *
 * Create a certain number of threads in the background and process data as it
 * is added to a queue.
 *
 * Example:
 *
 *   Output function(Input) { return Output(Input); }
 *   ThreadQueue<Output, Input> ts(function);
 *   ts.queue(Input);
 *   // other processing
 *   std::vector<Output> results = ts.results();
 */


#ifndef H_THREADQUEUE
#define H_THREADQUEUE

#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <condition_variable>

#include "cores.h"

// Thrown if attempting to add more items to the queue after all the threads
// have exited.
class ThreadsExited { };

// Each of the threads created, just waits for data in the queue and calls the
// function on an item when one is available.
template<class Result, class Item> class Thread
{
    Result (*function)(Item);
    std::condition_variable& moreData;
    std::vector<Result>& r;
    std::mutex& rMutex;
    std::queue<Item>& q;
    std::mutex& qMutex;
    std::atomic_bool& waiting;
    std::atomic_bool& killed;

public:
    Thread(Result (*function)(Item), std::condition_variable& moreData,
            std::vector<Result>& r, std::mutex& rMutex, std::queue<Item>& q,
            std::mutex& qMutex, std::atomic_bool& waiting, std::atomic_bool& killed)
        : function(function), moreData(moreData), r(r), rMutex(rMutex), q(q),
          qMutex(qMutex), waiting(waiting), killed(killed) { }

    void operator()();
};

template<class Result, class Item> class ThreadQueue
{
    // Our thread pool
    std::vector<std::thread> pool;

    // Are we waiting for the results now? If so, exit once the queue
    // becomes empty.
    std::atomic_bool waiting;

    // Did we already exit, don't allow adding more items to the queue
    std::atomic_bool killed;

    // Items to process
    std::queue<Item> q;
    std::mutex qMutex;

    // Results
    std::vector<Result> r;
    std::mutex rMutex;

    // Signal we have more data (wake up a thread to process)
    std::condition_variable moreData;

public:
    // Create a thread queue with a certain number of threads. Normally you'd
    // detect and specify the number of CPU cores in the computer. Defaults to
    // supposed number of cores if zero. Initialization does not block.
    ThreadQueue(Result (*function)(Item), int threads = 0);

    // Add another item to the queue to be processed and signal any waiting
    // threads that there's more data. Throws ThreadsExited() if already called
    // results().
    void queue(Item);

    // Block until all items in queue have been processed and return the return
    // values of the function. This will also exit all of the threads, so
    // adding anything more to the queue will throw an exception.
    std::vector<Result> results();

    // Quit processing new items in the queue
    void exit();
};

template<class Result, class Item> void Thread<Result,Item>::operator()()
{
    while (true)
    {
        Item item;

        {
            std::unique_lock<std::mutex> lck(qMutex);
            moreData.wait(lck, [this]{ return !q.empty() || waiting || killed; });

            if ((waiting && q.empty()) || killed)
                break;

            item = q.front();
            q.pop();
        }

        Result result = function(item);

        {
            std::lock_guard<std::mutex> lck(rMutex);
            r.push_back(result);
        }
    }
}


template<class Result, class Item> ThreadQueue<Result,Item>::ThreadQueue(
        Result (*function)(Item), int threads)
    : waiting(false), killed(false)
{
    if (threads <= 0)
        threads = core_count();

    for (int i = 0; i < threads; ++i)
        pool.push_back(std::thread(Thread<Result,Item>(function,
                    moreData, r, rMutex, q, qMutex, waiting, killed)));
}

template<class Result, class Item> void ThreadQueue<Result,Item>::queue(Item i)
{
    // Just to make sure we will actually process these eventually
    if (waiting)
        throw ThreadsExited();

    {
        std::lock_guard<std::mutex> lck(qMutex);
        q.push(i);
    }

    moreData.notify_one();
}

template<class Result, class Item> void ThreadQueue<Result,Item>::exit()
{
    killed = true;

    // Cause all other non-working threads to die
    moreData.notify_all();

    // Wait for these to exit
    for (std::thread& t : pool)
        t.join();
}

template<class Result, class Item> std::vector<Result> ThreadQueue<Result,Item>::results()
{
    // We want all threads to die as the queue becomes empty.
    waiting = true;

    // Cause all other non-working threads to die
    moreData.notify_all();

    // Wait for the results
    for (std::thread& t : pool)
        t.join();

    std::lock_guard<std::mutex> lck(rMutex); // Not really needed
    return r;
}

#endif
