#include <string>
#include <chrono>
#include <stdlib.h>
#include <cppgpio.hpp>

using namespace GPIO;

class RotaryDialWithPush
{
public:
    RotaryDialWithPush(std::function<void(bool, long)> *dialed, std::function<void()> *pushed, std::function<void(std::chrono::nanoseconds)> *released) 
        : dial(25, 9, GPIO_PULL::UP), push(11, GPIO_PULL::UP)
    {
        // register a lambda function at the dial to connect it to this class

        // dial.f_dialed = [&](bool up, long value)
        // { dialed(up, value); };
        dial.f_dialed = *dialed;

        // could also use std::bind():
        // dial.f_dialed = std::bind(&Rotary1::dialed, this, std::placeholders::_1, std::pslaceholders::_2);

        // push.f_pushed = [&]()
        // { pushed(); };

        // push.f_released = [&](std::chrono::nanoseconds nano)
        // { released(nano); };

        push.f_pushed = *pushed;

        push.f_released = *released;

        // after finishing the initialization of the event driven input objects
        // start the event threads of the input objects

        dial.start();
        push.start();
    }

private:
    RotaryDial dial;
    PushButton push;
};