#ifndef SCREEN_MENU_H
#define SCREEN_MENU_H
#include "screen.h"
#include "scrolling-line.h"
#include "screen_state.h"

class ScreenMenu : public Screen
{
public:
    ScreenMenu(ScrollingLineSettings line1_settings, ScrollingLineSettings line2_settings, ScreenState *state);
    void render(FrameCanvas *offscreen_canvas);
    void scrollMenu(bool up);
    void modeChange();
private:
    ScrollingLine menu_line;
    ScrollingLine menu_sub_line;
    ScreenState *state;
    int current_menu_item;
            std::vector<std::string> menu_items;

};
#endif /*SCREEN_MENU_H*/