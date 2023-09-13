#ifndef SCREEN_MENU_H
#define SCREEN_MENU_H
#include "screen.h"
#include "scrolling-line.h"
#include "screen_state.h"
#include <cppgpio.hpp>

class ScreenMenu : public Screen
{
public:
    ScreenMenu(float speed,
               int letter_spaceing,
               Font *font,
               int screen_width,
               ScreenState *state,
               GPIO::RotaryDial *dial,
               GPIO::PushButton *button);
    void render(FrameCanvas *offscreen_canvas);

private:
    void scrollMenu(bool up);
    void modeChange();
    enum MenuMode {main_menu, brightness_menu, switch_order_menu};
    MenuMode current_mode = main_menu;
    ScrollingLine menu_line;
    ScrollingLine menu_sub_line;
    ScreenState *state;
    int current_menu_item;
    std::vector<std::string> menu_items;
};
#endif /*SCREEN_MENU_H*/