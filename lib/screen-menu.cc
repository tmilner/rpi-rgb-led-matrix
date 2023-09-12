#include "screen-menu.h"
#include <iostream>
#include "img_utils.h"

ScreenMenu::ScreenMenu(ScrollingLineSettings line1_settings, ScrollingLineSettings line2_settings, ScreenState *state)
{
    this->menu_sub_line = ScrollingLine(line2_settings);
    this->menu_line = ScrollingLine(line1_settings);
    this->state = state;
}

void ScreenMenu::render(FrameCanvas *offscreen_canvas)
{
    offscreen_canvas->SetBrightness(this->state->current_brightness);

    if (this->state->current_mode == main_menu)
    {
        offscreen_canvas->SetPixels(0, 7, offscreen_canvas->width(), offscreen_canvas->height()- 13, 50, 50, 50);
    }
    else
    {
        offscreen_canvas->SetPixels(0, 7, offscreen_canvas->width(), offscreen_canvas->height() - 13, 233, 110, 80);
        offscreen_canvas->SetPixels(1, 8, offscreen_canvas->width() - 2, offscreen_canvas->height() - 15, 50, 50, 50);
        menu_sub_line.updateText(&std::to_string(this->state->current_brightness).append("%"));
        menu_sub_line.renderLine(offscreen_canvas);
    }
    menu_line.updateText(&this->state->menu_items[this->state->current_menu_item]);
    menu_line.renderLine(offscreen_canvas);
}
