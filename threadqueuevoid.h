/*
 * Function returns void version.
 *
 * Create a certain number of threads in the background and process data as it
 * is added to a queue.
 *
 * Example:
 *
 *   void function(Input) { }
 *   ThreadQueueVoid<Input> ts(function);
 *   ts.queue(Input);
 *   // other processing
 *   ts.wait();
 */


#ifndef H_THREADQUEUEVOID
#define H_THREADQUEUEVOID

#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <condition_variable>

#include "cores.h"

// Thrown if attempting to add more items to the queue after all the threads
// have exited.
class ThreadsExitedVoid { };

// Each of the threads created, just waits for data in the queue and calls the
// function on an item when one is available.
template<class Item> class ThreadVoid
{
    void (*function)(Item);
    std::condition_variable& moreData;
    std::queue<Item>& q;
    std::mutex& qMutex;
    std::atomic_bool& waiting;
    std::atomic_bool& killed;

public:
    ThreadVoid(void (*function)(Item), std::condition_variable& moreData,
            std::queue<Item>& q, std::mutex& qMutex, std::atomic_bool& waiting,
            std::atomic_bool& killed)
        : function(function), moreData(moreData), q(q), qMutex(qMutex),
        waiting(waiting), killed(killed) { }

    void operator()();
};

template<class Item> class ThreadQueueVoid
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

    // Signal we have more data (wake up a thread to process)
    std::condition_variable moreData;

public:
    // Create a thread queue with a certain number of threads. Normally you'd
    // detect and specify the number of CPU cores in the computer. Defaults to
    // supposed number of cores if zero. Initialization does not block.
    ThreadQueueVoid(void (*function)(Item), int threads = 0);

    // Add another item to the queue to be processed and signal any waiting
    // threads that there's more data. Throws ThreadsExited() if already called
    // results().
    void queue(Item);

    // Block until all items in queue have been processed. This will also exit
    // all of the threads, so adding anything more to the queue will throw an
    // exception.
    void wait();

    // Quit processing new items in the queue
    void exit();
};

template<class Item> void ThreadVoid<Item>::operator()()
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

        function(item);
    }
}


template<class Item> ThreadQueueVoid<Item>::ThreadQueueVoid(
        void (*function)(Item), int threads)
    : waiting(false), killed(false)
{
    if (threads <= 0)
        threads = core_count();

    for (int i = 0; i < threads; ++i)
        pool.push_back(std::thread(ThreadVoid<Item>(function,
                    moreData, q, qMutex, waiting, killed)));
}

template<class Item> void ThreadQueueVoid<Item>::queue(Item i)
{
    // Just to make sure we will actually process these eventually
    if (waiting)
        throw ThreadsExitedVoid();

    {
        std::lock_guard<std::mutex> lck(qMutex);
        q.push(i);
    }

    moreData.notify_one();
}

template<class Item> void ThreadQueueVoid<Item>::exit()
{
    killed = true;

    // Cause all other non-working threads to die
    moreData.notify_all();

    // Wait for these to exit
    for (std::thread& t : pool)
        t.join();
}

template<class Item> void ThreadQueueVoid<Item>::wait()
{
    // We want all threads to die as the queue becomes empty.
    waiting = true;

    // Cause all other non-working threads to die
    moreData.notify_all();

    // Wait for the results
    for (std::thread& t : pool)
        t.join();
}

#endif
