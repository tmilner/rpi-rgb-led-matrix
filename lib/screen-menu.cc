#include "screen-menu.h"
#include <iostream>
#include "img_utils.h"

ScreenMenu::ScreenMenu(float speed, int letter_spaceing, Font *font, int screen_width, ScreenState *state,
                       GPIO::RotaryDial *dial,
                       GPIO::PushButton *button)
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
          0)}
{
    this->is_visible = false;
    this->state = state;
    this->current_menu_item = 1;
    this->menu_items = {"Brightness", "Switch Order", "Exit"};

    dial->f_dialed = [&](bool up, long value)
    { this->scrollMenu(up); };

    button->f_pushed = [&]()
    { this->modeChange(); };

    dial->start();
    button->start();
}
void ScreenMenu::scrollMenu(bool up)
{

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
    else if (this->current_mode == MenuMode::switch_order_menu)
    {
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
        this->current_mode = MenuMode::switch_order_menu;
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
    std::cout << "Render Menu" << std::endl;
    if (this->current_mode == main_menu)
    {
        offscreen_canvas->SetPixels(0, 7, offscreen_canvas->width(), offscreen_canvas->height() - 13, 50, 50, 50);
    }
    else
    {
        offscreen_canvas->SetPixels(0, 7, offscreen_canvas->width(), offscreen_canvas->height() - 13, 233, 110, 80);
        offscreen_canvas->SetPixels(1, 8, offscreen_canvas->width() - 2, offscreen_canvas->height() - 15, 50, 50, 50);
        menu_sub_line.updateText(&std::to_string(this->state->current_brightness).append("%"));
        menu_sub_line.renderLine(offscreen_canvas);
    }
    menu_line.updateText(&this->menu_items[this->current_menu_item]);
    menu_line.renderLine(offscreen_canvas);
}
