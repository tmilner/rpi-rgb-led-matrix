#include "led-matrix.h"
#include "graphics.h"

#include <string>
#include "screen-line.h"

using namespace rgb_matrix;

ScreenLine::ScreenLine(int init_speed, int init_x, int init_y, int init_letter_spacing, Font *init_font, Color init_color, std::string *startingLine)
{
    currentLine = *startingLine;
    x_orig = init_x;
    x = init_x;
    y = init_y;
    length = 0;
    letter_spacing = init_letter_spacing;
    color = init_color;
    font = *init_font;
    speed = init_speed;
};
void ScreenLine::updateText(std::string *newLineString)
{
    currentLine = *newLineString;
};
void ScreenLine::render(FrameCanvas *offscreen_canvas)
{
    length = rgb_matrix::DrawText(offscreen_canvas, font,
                                  x, y + font.baseline(),
                                  color, nullptr,
                                  currentLine.c_str(), letter_spacing);

    if (speed > 0 && --x + length < 0)
    {
        x = x_orig;
    }
};