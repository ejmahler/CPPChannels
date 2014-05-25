CPPChannels
===========
This is a simple implementation of go channels in C++, using all sorts of fancy C++11 features. A channel is essentially a queue designed for inter-thread communcation.

The `Channel` class is contained within `channel.h`. `main.cpp` creates a demonstration of several threads adding and removing items from channels under various configurations.

Usage
----------
At its core, a channel operates similar to a std::queue. It has a push method(`Channel<T>::push`), a pop method(`Channel<T>::pop`), and items are popped in FIFO order.

The primary intended use case is one or more threads (the "producers") pushing items into a channel, and one or more threads (the "consumers") popping them out.

When a consumer tries to pop an item from an empty channel, the consumer thread will block until an item becomes available.

#### Closing
When the producer is done producing items, it closes the channel (`Channel<T>::closeBack`), indicating to the consumer(s) that no more items will be added to the channel. Any items inside the channel when the channel is closed can still be consumed, but is an error to push an item into a channel whose back is closed.

In addition to closing the back of the channel, the front of the channel can also be closed. The intended use case is for use by a consumer that needs to indicate to the producer that no more items will be consumed, and that the producer should stop producing.

#### Full Behavior
A channel can have a maximum size, specified in the `Channel `constructor. Trying to push an item onto a full channel results in different behavior depending on what you provided for the fullPushBehavior field in the Channel constructor:
* **BLOCK**: Block the current thread until another thread pops something.
* **DROP_NEWEST**: Silently discard the newest item in the queue (IE silently discard the item that the caller is currently attempting to push)
* **DROP_OLDEST**: Silently discard the oldest item in the queue (IE silently discard "front" of the queue)
* **NEVER_FULL**(default) Ignore the max_size and behave as if the queue does not have a maximum size.

Building
----------
To build the demo, open CPPChannels.pro in Qt Creator, or use qmake to generate a makefile.

License
----------
[MIT License](http://opensource.org/licenses/MIT)
