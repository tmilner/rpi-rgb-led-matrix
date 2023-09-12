#include "screen.h"
#include "scrolling-line.h"
#include "screen_state.h"

class ScreenMenu : public Screen
{
public:
    ScreenMenu(ScrollingLineSettings line1_settings, ScrollingLineSettings line2_settings, ScreenState *state);
    void render(FrameCanvas *offscreen_canvas);

private:
    ScrollingLine menu_line;
    ScrollingLine menu_sub_line;
    ScreenState *state;
};
