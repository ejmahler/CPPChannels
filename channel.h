#ifndef CHANNEL_H
#define CHANNEL_H

#include <cassert>

#include <queue>

#include <mutex>
#include <condition_variable>
#include <atomic>

#include <iostream>

template<class T>
class Channel
{
public:
    enum FullPushBehavior { BLOCK, DROP_NEWEST, DROP_OLDEST, NEVER_FULL };
    Channel(void);
    Channel(FullPushBehavior fullPushBehavior, size_t maxSize = 1);
    Channel(const Channel &other) = delete;

    bool isClosed(void) const;
    void close(void);

    void push(const T& item);
    bool pop(T& result);

    //Channel<T>& operator<< (const T& item);
    //Channel<T>& operator>> (T& item);

private:
    const FullPushBehavior fullPushBehavior;
    const size_t maxSize;
    std::atomic<bool> _isClosed;

    std::queue<T> queue;
    std::mutex queueMutex;
    std::condition_variable fullWait;
    std::condition_variable emptyWait;
};

class ChannelClosedException : public std::exception
{

};

template<class T>
Channel<T>::Channel(void)
    :fullPushBehavior(NEVER_FULL), maxSize(0), _isClosed(false)
{

}

template<class T>
Channel<T>::Channel(FullPushBehavior fullPushBehavior, size_t maxSize)
    :fullPushBehavior(fullPushBehavior), maxSize(maxSize), _isClosed(false)
{
    if(fullPushBehavior != NEVER_FULL)
        assert(maxSize > 0);
}

template<class T>
bool Channel<T>::isClosed(void) const
{
    return _isClosed.load(std::memory_order_acquire);
}

template<class T>
void Channel<T>::close(void)
{
    _isClosed.store(true, std::memory_order_release);

    //if anyone is waiting for the "empty" condition variable, wake them up so that they know they're not going to get any more data
    emptyWait.notify_all();
}

template<class T>
void Channel<T>::push(const T& item)
{
    if(isClosed())
    {
        throw ChannelClosedException();
    }

    std::unique_lock<std::mutex> locker(queueMutex);

    //set up a lambda that returns true if the queue is not full
    auto canAddItem = [this]() { return queue.size() < maxSize; };

    if(fullPushBehavior == NEVER_FULL)
    {
        queue.push(item);

        //wake up anyone who might be waiting
        emptyWait.notify_one();
    }
    else if(fullPushBehavior == BLOCK)
    {
        //block until the queue isn't full anymore
        if(!canAddItem())
        {
            fullWait.wait(locker, canAddItem);
        }
        queue.push(item);

        //wake up anyone who might be waiting
        emptyWait.notify_one();
    }
    else if(fullPushBehavior == DROP_NEWEST)
    {
        //if the queue is full, we're just going to drop the given item
        if(canAddItem())
        {
            queue.push(item);

            //wake up anyone who might be waiting
            emptyWait.notify_one();
        }
    }
    else if(fullPushBehavior == DROP_OLDEST)
    {
        //if the queue is full, pop off the front of the channel
        if(!canAddItem())
        {
            queue.pop();
        }
        queue.push(item);

        //wake up anyone who might be waiting
        emptyWait.notify_one();
    }
}

template<class T>
bool Channel<T>::pop(T& result)
{
    std::unique_lock<std::mutex> locker(queueMutex);

    //if the queue is closed and empty, return false to iindicate that the caller will not recieve a response
    if(isClosed() && queue.empty())
        return false;

    //if the queue is empty, block!
    if(queue.empty())
    {
        emptyWait.wait(locker, [this]() { return isClosed() || !queue.empty(); });

        //if the queue is still empty, it means it's closed and will never get another item,so return false
        if(queue.empty())
            return false;
    }

    result = queue.front();
    queue.pop();

    //notify any pushers who may be waiting on a full queue that it is no longer empty
    fullWait.notify_one();
    return true;
}

#endif // CHANNEL_H
