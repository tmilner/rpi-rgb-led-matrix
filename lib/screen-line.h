#include "led-matrix.h"
#include "graphics.h"
#include <string>

using namespace rgb_matrix;

class ScreenLine
{
    std::string currentLine;
    int x;
    int y;
    int length;
    int letter_spacing;
    int speed;
    int orig_speed;
    int icon_offset;
    int screen_width;
    int max_width_for_no_scrolling;
    rgb_matrix::Font font;
    Color color;

public:
    ScreenLine(int init_speed, int init_y, int init_letter_spacing, Font *init_font, Color init_color, std::string *starting_line, int init_screen_width, int init_icon_offset);
    void updateText(std::string &newLineString);
    void render(FrameCanvas *offscreen_canvas);
};