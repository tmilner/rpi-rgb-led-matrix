#include <string>
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <cppgpio.hpp>
#include "screen_mode.h"

using namespace GPIO;

class RotaryDialWithPush
{
public:
    RotaryDialWithPush(ScreenMode &current_mode, int &current_menu_item, std::vector<std::string> &menu_items, int &current_brightness)
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

private:
    RotaryDial dial;
    PushButton push;

    void dialed(bool up, long value, ScreenMode &mode, int &menu_item, std::vector<std::string> &menu_items, int &brightness)
    {
        if (mode == ScreenMode::main_menu)
        {

            if (!up)
            {
                menu_item++;
                std::cout << "Main Menu. Scroll up!!" << menu_item << ", Count == " << menu_items.size() << std::endl;

                if (menu_item >= menu_items.size())
                {
                    menu_item = 0;
                }
            }
            else
            {
                menu_item--;
                std::cout << "Main Menu. Scroll down!!" << menu_item << ", Count == " << menu_items.size() << std::endl;

                if (menu_item < 0)
                {
                    menu_item = (menu_items.size() - 1);
                }
            }
        }
        else if (mode == ScreenMode::brightness_menu)
        {
            if (!up)
            {
                std::cout << "Brightness Menu. Scroll up!!" << brightness << std::endl;

                if (brightness != 100)
                {
                    brightness += (value * 2);
                    if (brightness > 100)
                    {
                        brightness = 100;
                    }
                }
            }
            else
            {
                std::cout << "Brightness Menu. Scroll down!!" << brightness << std::endl;

                if (brightness != 0)
                {
                    brightness -= (value * 2);
                    if (brightness < 0)
                    {
                        brightness = 0;
                    }
                }
            }
        }
        return;
    }

    void pushed(ScreenMode &mode, int &menu_item)
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
    }

    void released(std::chrono::nanoseconds nano)
    {
        return;
    }
};