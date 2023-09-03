#include "led-matrix.h"
#include "graphics.h"
#include <string>

using namespace rgb_matrix;

class ScreenLine
{
    std::string currentLine;
    int x_orig;
    int x;
    int y;
    int length;
    int letter_spacing;
    int speed;
    rgb_matrix::Font font;
    Color color;

public:
    ScreenLine(int init_speed, int init_x, int init_y, int init_letter_spacing, Font *init_font, Color init_color, std::string *startingLine);
    void updateText(std::string *newLineString);
    void render(FrameCanvas *offscreen_canvas);
};