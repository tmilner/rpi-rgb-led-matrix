#include "rotary_dial.h"
#include <iostream>

using namespace GPIO;

RotaryDialWithPush::RotaryDialWithPush(ScreenMode &current_mode, int &current_menu_item, std::vector<std::string> &menu_items, int &current_brightness)
    : dial(25, 9, GPIO_PULL::UP), push(11, GPIO_PULL::UP)
{
    // register a lambda function at the dial to connect it to this class
    dial.f_dialed = [&](bool up, long value)
    { dialed(up, value, current_mode, current_menu_item, menu_items, current_brightness); };

    push.f_pushed = [&]()
    { pushed(current_mode, current_menu_item); };

    push.f_released = [&](std::chrono::nanoseconds nano)
    { released(nano); };

    // after finishing the initialization of the event driven input objects
    // start the event threads of the input objects

    dial.start();
    push.start();
}

void RotaryDialWithPush::dialed(bool up, long value, ScreenMode &mode, int &menu_item, std::vector<std::string> &menu_items, int &brightness)
{
    if (mode == ScreenMode::main_menu)
    {

        if (up)
        {
            std::cout << "Main Menu. Scroll up!!" << menu_item << ", Count == " << menu_items.size() << std::endl;

            if (menu_item + 1 >= menu_items.size())
            {
                menu_item = 0;
            }
            else
            {
                menu_item++;
            }
        }
        else
        {
            std::cout << "Main Menu. Scroll down!!" << menu_item << ", Count == " << menu_items.size() << std::endl;

            if (menu_item = 0)
            {
                menu_item = (menu_items.size() - 1);
            }
            else
            {
                menu_item--;
            }
        }
    }
    else if (mode == ScreenMode::brightness_menu)
    {
        if (up)
        {
            std::cout << "Brightness Menu. Scroll up!!" << brightness << " " << value << std::endl;

            if (brightness != 100)
            {
                int increment_by = 5;
                if (brightness + increment_by > 100)
                {
                    brightness = 100;
                }
                else
                {
                    brightness += increment_by;
                }
            }
        }
        else
        {
            std::cout << "Brightness Menu. Scroll down!!" << brightness << " " << value << std::endl;

            if (brightness != 0)
            {
                int decrement_by = 5;
                if ((brightness - decrement_by) < 0)
                {
                    brightness = 0;
                }
                else
                {
                    brightness -= decrement_by;
                }
            }
        }
    }
    return;
};

void RotaryDialWithPush::pushed(ScreenMode &mode, int &menu_item)
{
    std::cout << "Pressed! Current Mode " << mode << std::endl;
    if (mode == ScreenMode::display)
    {
        std::cout << "pressed go to menu" << std::endl;
        mode = ScreenMode::main_menu;
    }
    else if (mode == ScreenMode::main_menu && menu_item == 0)
    {
        std::cout << "pressed go to brightness" << std::endl;
        mode = ScreenMode::brightness_menu;
    }
    else
    {
        std::cout << "pressed go to display" << std::endl;
        mode = ScreenMode::display;
    }
    return;
};

void RotaryDialWithPush::released(std::chrono::nanoseconds nano)
{
    return;
};