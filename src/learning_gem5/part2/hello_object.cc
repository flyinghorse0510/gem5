#include "learning_gem5/part2/hello_object.hh"
#include "base/trace.hh"
#include "debug/Hello.hh"
#include <iostream>

namespace gem5
{

    HelloObject::HelloObject(const HelloObjectParams &params):
        SimObject(params), event([this]{processEvent();}, name()),
        latency(params.time_to_wait), timesLeft(params.number_of_fires),
        goodbye(params.goodbye_object)
    {
        DPRINTF(Hello, "Created the hello object with the name %s\n", name());
        panic_if(!goodbye, "HelloObject must have a non-null GoodbyeObject");
    }

    void
    HelloObject::processEvent() {
        timesLeft--;
        DPRINTF(Hello, "Hello world! Processing the event! %d left\n", timesLeft);

        if (timesLeft <= 0) {
            DPRINTF(Hello, "Done firing!\n");
            goodbye->sayGoodbye(name());
        } else {
            schedule(event, curTick() + latency);
        }
    }

    void
    HelloObject::startup() {
        schedule(event, latency);
    }
}