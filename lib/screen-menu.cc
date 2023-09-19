#include "screen-menu.h"
#include <iostream>
#include "img_utils.h"

ScreenMenu::ScreenMenu(float speed, int letter_spaceing, Font *font, int screen_width, ScreenState *state,
                       GPIO::PushButton *button_ok, GPIO::PushButton *button_up, GPIO::PushButton *button_down, std::vector<Screen *> *screens)
    : menu_line{ScrollingLineSettings(
          speed,
          9,
          letter_spaceing,
          font,
          Color(240, 160, 100),
          screen_width,
          0)},
      menu_sub_line{ScrollingLineSettings(
          speed,
          17,
          letter_spaceing,
          font,
          Color(240, 160, 100),
          screen_width,
          0)},
      screens{screens}
{
    this->is_visible = false;
    this->state = state;
    this->current_menu_item = 1;
    this->current_screen = 0;
    this->menu_items = {"Brightness", "Switch Screen", "Exit"};
    this->last_button_press = std::chrono::system_clock::now();

    button_ok->f_released = [&](std::chrono::nanoseconds nano)
    { this->modeChange(); };

    button_up->f_released = [&](std::chrono::nanoseconds nano)
    { this->scrollMenu(true); };

    button_down->f_released = [&](std::chrono::nanoseconds nano)
    { this->scrollMenu(false); };

    button_ok->start();
    button_up->start();
    button_down->start();
}
bool ScreenMenu::debounceTimePassed()
{
    using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

    const auto now = std::chrono::system_clock::now();

    if (((now - this->last_button_press) / 1ms) < 100)
    {
        return false;
    }
    else
    {
        this->last_button_press = now;
        return true;
    }
}
void ScreenMenu::scrollMenu(bool up)
{
    if (!debounceTimePassed())
    {
        return;
    }

    if (this->current_mode == MenuMode::main_menu)
    {

        if (up)
        {
            std::cout << "Main Menu. Scroll up!!" << this->current_menu_item << ", Count == " << menu_items.size() << std::endl;

            if (this->current_menu_item + 1 >= this->menu_items.size())
            {
                this->current_menu_item = 0;
            }
            else
            {
                this->current_menu_item++;
            }
        }
        else
        {
            std::cout << "Main Menu. Scroll down!!" << this->current_menu_item << ", Count == " << menu_items.size() << std::endl;

            if (this->current_menu_item = 0)
            {
                this->current_menu_item = (this->menu_items.size() - 1);
            }
            else
            {
                this->current_menu_item--;
            }
        }
    }
    else if (this->current_mode == MenuMode::switch_screen)
    {
        if (up)
        {
            if (this->current_screen + 1 > this->screens->size())
            {
                this->current_screen = 0;
            }
            else
            {
                this->current_screen++;
            }
        }
        else
        {
            if (this->current_screen - 1 < 0)
            {
                this->current_screen = this->screens->size() - 1;
            }
            else
            {
                this->current_screen--;
            }
        }
    }
    else if (this->current_mode == MenuMode::brightness_menu)
    {
        if (up)
        {
            std::cout << "Brightness Menu. Scroll up!!" << this->state->current_brightness << std::endl;

            if (this->state->current_brightness != 100)
            {
                int increment_by = 5;
                if (this->state->current_brightness + increment_by > 100)
                {
                    this->state->current_brightness = 100;
                }
                else
                {
                    this->state->current_brightness += increment_by;
                }
            }
        }
        else
        {
            std::cout << "Brightness Menu. Scroll down!!" << this->state->current_brightness << std::endl;

            if (this->state->current_brightness != 0)
            {
                int decrement_by = 5;
                if (this->state->current_brightness - decrement_by < 0)
                {
                    this->state->current_brightness = 0;
                }
                else
                {
                    this->state->current_brightness -= decrement_by;
                }
            }
        }
    }
}

void ScreenMenu::modeChange()
{
    if (!debounceTimePassed())
    {
        return;
    }
    std::cout << "Pressed! Current Mode " << this->state->current_mode << std::endl;
    if (!this->is_visible)
    {
        std::cout << "pressed go to menu" << std::endl;
        this->is_visible = true;
        this->current_mode = MenuMode::main_menu;
    }
    else if (this->current_mode == MenuMode::main_menu && this->current_menu_item == 0)
    {
        std::cout << "pressed go to brightness" << std::endl;
        this->current_mode = MenuMode::brightness_menu;
    }
    else if (this->current_mode == MenuMode::main_menu && this->current_menu_item == 1)
    {
        std::cout << "pressed go to switch order menu" << std::endl;
        this->current_mode = MenuMode::switch_screen;
        this->current_screen = 0;
    }
    else if (this->current_mode == MenuMode::switch_screen)
    {
        std::cout << "pressed go to change screen" << std::endl;

        int x = 0;
        for (std::vector<Screen *>::iterator screen = this->screens->begin(); screen != this->screens->end(); screen++)
        {
            if (x++ != this->current_screen)
                (*screen)->set_hidden();
            else
                (*screen)->set_visible();
        }

        this->is_visible = false;
        this->current_mode = MenuMode::main_menu;
    }
    else
    {
        std::cout << "pressed go to leave menu" << std::endl;
        this->is_visible = false;
        this->current_mode = MenuMode::main_menu;
    }
    return;
}

void ScreenMenu::render(FrameCanvas *offscreen_canvas)
{
    if (!this->is_visible)
    {
        return;
    }
    if (this->current_mode == main_menu)
    {
        offscreen_canvas->SetPixels(0, 7, offscreen_canvas->width(), offscreen_canvas->height() - 13, 50, 50, 50);
    }
    else
    {
        offscreen_canvas->SetPixels(0, 7, offscreen_canvas->width(), offscreen_canvas->height() - 13, 233, 110, 80);
        offscreen_canvas->SetPixels(1, 8, offscreen_canvas->width() - 2, offscreen_canvas->height() - 15, 50, 50, 50);
        if (this->current_mode == brightness_menu)
        {
            menu_sub_line.updateText(&std::to_string(this->state->current_brightness).append("%"));
        }
        else if (this->current_mode == switch_screen)
        {
            menu_sub_line.updateText(this->screens->at(this->current_screen)->getName());
        }
        menu_sub_line.renderLine(offscreen_canvas);
    }
    menu_line.updateText(&this->menu_items[this->current_menu_item]);
    menu_line.renderLine(offscreen_canvas);
}
