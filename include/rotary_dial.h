#include <string>
#include <chrono>
#include <stdlib.h>
#include <cppgpio.hpp>
#include "screen_mode.h"
#include "screen-menu.h"

using namespace GPIO;

class RotaryDialWithPush
{
public:
    RotaryDialWithPush(ScreenMenu &menu);

private:
    RotaryDial dial;
    PushButton push;
    void dialed(bool up, long value, ScreenMenu &menu);
    void pushed(ScreenMenu &menu);
    void released(std::chrono::nanoseconds nano);
};