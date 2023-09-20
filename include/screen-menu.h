#ifndef SCREEN_MENU_H
#define SCREEN_MENU_H
#include "screen.h"
#include "scrolling-line.h"
#include "screen_state.h"
#include <cppgpio.hpp>
#include <chrono>

class ScreenMenu : public Screen
{
public:
    ScreenMenu(float speed,
               int letter_spaceing,
               Font *font,
               int screen_width,
               ScreenState *state,
               GPIO::PushButton *button_ok,
               GPIO::PushButton *button_up,
               GPIO::PushButton *button_down,
               std::vector<Screen *> *screens);
    void render(FrameCanvas *offscreen_canvas);
    std::string *getName();

private:
    void scrollMenu(bool up);
    void modeChange();
    bool debounceTimePassed();
    std::string name;
    std::vector<Screen *> *screens;
    enum MenuMode
    {
        main_menu,
        brightness_menu,
        switch_screen
    };
    MenuMode current_mode = main_menu;
    ScrollingLine menu_line;
    ScrollingLine menu_sub_line;
    ScreenState *state;
    int current_menu_item;
    int current_screen;
    std::vector<std::string> menu_items;
    std::chrono::time_point<std::chrono::system_clock> last_button_press;
};
#endif /*SCREEN_MENU_H*/