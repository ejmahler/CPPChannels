CPPChannels
===========
This is a simple implementation of go channels in C++, using all sorts of fancy C++11 features. A channel is essentially a queue designed for inter-thread communcation.

The `Channel` class is contained within `channel.h`. `main.cpp` creates a demonstration of several threads adding and removing items from channels under various configurations.

Usage
----------
The most basic use case is one thread (the "producer") pushing items into a channel, and another thread (the "consumer") pulling them out. When the producing thread is done, it `closes` the channel, indicating to the consumer that no more items will be added to the channel. Any items inside the channel when the channel is closed can still be consumed, but is an error to push an item into a channel which is closed.

Trying to pop an item off of an empty channel will block the current thread until an item is available or the channel is closed.

A channel can have a maximum size, specified in the `Channel `constructor. Trying to push an item onto a full channel resulte in different behavior depending on what you provided for the fullPushBehavior field in the Channel constructor:
* **BLOCK**: Block the current thread until another thread pops something.
* **DROP_NEWEST**: Silently discard the newest item in the queue (IE silently discard the item that the caller is currently attempting to push)
* **DROP_OLDEST**: Silently discard the oldest item in the queue (IE silently discard "front" of the queue)
* **NEVER_FULL**(default) Ignore the max_size and behave as if the queue does not have a maximum size.

Building
----------
Open CPPChannels.pro in Qt Creator, or use qmake to generate a makefile.

License
----------
[MIT License](http://opensource.org/licenses/MIT)
