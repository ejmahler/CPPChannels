#include <iostream>
#include <sstream>
#include <future>
#include <thread>

#include <boost/format.hpp>

#include "channel.h"

using boost::str;
using boost::format;

int channelSize = 40;

void sleep(int msec)
{
    std::chrono::milliseconds duration(msec);
    std::this_thread::sleep_for(duration);
}

void performDemo(Channel<int> *dataChannel, Channel<std::string> *outputChannel);
void producerFunction(Channel<int> *dataChannel, Channel<std::string> *outputChannel);
void consumerFunction(Channel<int> *dataChannel, Channel<std::string> *outputChannel, int consumerNum);

int main()
{
    std::cout << "Doing a demo with a blocking channel" << std::endl;
    {
        Channel<std::string> outputChannel;
        Channel<int> dataChannel(Channel<int>::BLOCK, channelSize);
        performDemo(&dataChannel, &outputChannel);
    }

    std::cout << std::endl << "Doing a demo with a 'delete oldest' channel" << std::endl;
    {
        Channel<std::string> outputChannel;
        Channel<int> dataChannel(Channel<int>::DROP_OLDEST, channelSize);
        performDemo(&dataChannel, &outputChannel);
    }

    std::cout << std::endl << "Doing a demo with a 'delete newest' channel" << std::endl;
    {
        Channel<std::string> outputChannel;
        Channel<int> dataChannel(Channel<int>::DROP_NEWEST, channelSize);
        performDemo(&dataChannel, &outputChannel);
    }

    return 0;
}

void performDemo(Channel<int> *dataChannel, Channel<std::string> *outputChannel)
{
    auto outputFuture = std::async(
        std::launch::async,
        [=]() {
            //pop output strings from the output channel and put them in cout
            std::string outputResult;
            while(outputChannel->pop(outputResult))
            {
                std::cout << outputResult << std::endl;
            }
        });

    auto producerFuture = std::async(std::launch::async, producerFunction, dataChannel, outputChannel);
    auto consumerFuture1 = std::async(std::launch::async, consumerFunction, dataChannel, outputChannel, 1);
    auto consumerFuture2 = std::async(std::launch::async, consumerFunction, dataChannel, outputChannel, 2);
    auto consumerFuture3 = std::async(std::launch::async, consumerFunction, dataChannel, outputChannel, 3);
    auto consumerFuture4 = std::async(std::launch::async, consumerFunction, dataChannel, outputChannel, 4);

    producerFuture.wait();
    consumerFuture1.wait();
    consumerFuture2.wait();
    consumerFuture3.wait();
    consumerFuture4.wait();

    outputChannel->closeBack();
    outputFuture.wait();
}

void producerFunction(Channel<int> *dataChannel, Channel<std::string> *outputChannel)
{
    sleep(10);

    int numSlow = channelSize;
    int numFast = channelSize * 2;

    outputChannel->push(str(format("[PRODUCER] About to quickly produce %d items") % numFast));
    for(int i = 0; i < numFast; i++)
    {
        sleep(1);

        dataChannel->push(i);
        outputChannel->push(str(format("[PRODUCER] Just produced item %d") % i));
    }

    outputChannel->push(str(format("[PRODUCER] About to slowly produce %d items") % numSlow));
    for(int i = numFast; i < numFast + numSlow; i++)
    {
        sleep(50);
        dataChannel->push(i);

        outputChannel->push(str(format("[PRODUCER] Just produced item %d") % i));
    }
    dataChannel->closeBack();
}

void consumerFunction(Channel<int> *dataChannel, Channel<std::string> *outputChannel, int consumerNum)
{
    std::string consumerName = str(format("[CONSUMER %d]") % consumerNum);

    outputChannel->push(str(format("%s about to begin consuming") % consumerName));

    int data;
    while(dataChannel->pop(data))
    {
        outputChannel->push(str(format("%s Just consumed item %d") % consumerName % data));

        sleep(25);
    }
    outputChannel->push(str(format("%s done consuming") % consumerName));
}


