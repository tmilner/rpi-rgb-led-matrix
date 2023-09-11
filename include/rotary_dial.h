#include <string>
#include <chrono>
#include <stdlib.h>
#include <cppgpio.hpp>
#include "screen_mode.h"

using namespace GPIO;

class RotaryDialWithPush
{
public:
    RotaryDialWithPush(ScreenMode &current_mode, int &current_menu_item, std::vector<std::string> &menu_items, int &current_brightness);

private:
    RotaryDial dial;
    PushButton push;

    void dialed(bool up, long value, ScreenMode &mode, int &menu_item, std::vector<std::string> &menu_items, int &brightness);
    void pushed(ScreenMode &mode, int &menu_item);
    void released(std::chrono::nanoseconds nano);
};