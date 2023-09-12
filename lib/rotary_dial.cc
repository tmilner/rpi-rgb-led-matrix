#include "rotary_dial.h"
#include <iostream>

using namespace GPIO;

RotaryDialWithPush::RotaryDialWithPush(ScreenMenu &menu)
    : dial(25, 9, GPIO_PULL::UP), push(11, GPIO_PULL::UP)
{
    // register a lambda function at the dial to connect it to this class
    dial.f_dialed = [&](bool up, long value)
    { dialed(up, value, menu); };

    push.f_pushed = [&]()
    { pushed(menu); };

    push.f_released = [&](std::chrono::nanoseconds nano)
    { released(nano); };

    // after finishing the initialization of the event driven input objects
    // start the event threads of the input objects

    dial.start();
    push.start();
}

void RotaryDialWithPush::dialed(bool up, long value, ScreenMenu &menu)
{
    menu.scrollMenu(up);
    return;
};

void RotaryDialWithPush::pushed(ScreenMenu &menu)
{
menu.modeChange();
};

void RotaryDialWithPush::released(std::chrono::nanoseconds nano)
{
    return;
};