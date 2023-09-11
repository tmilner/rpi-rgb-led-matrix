#include "scrolling-line.h"
#include "led-matrix.h"
#include "graphics.h"

#include <string>
#include <iostream>
using namespace rgb_matrix;

ScrollingLine::ScrollingLine(ScrollingLineSettings settings)
{
    currentLine = "Loading";
    x = settings.init_icon_offset;
    y = settings.init_y;
    length = 0;
    letter_spacing = settings.init_letter_spacing;
    color = settings.init_color;
    font = *settings.init_font;
    orig_speed = settings.init_speed;
    speed = settings.init_speed;
    icon_offset = settings.init_icon_offset;
    screen_width = settings.init_screen_width;
    max_width_for_no_scrolling = (settings.init_screen_width - icon_offset);

    if (speed == 0)
    {
        x = 14;
    }

    std::cout << "Line created! Max width for scrolling = " << max_width_for_no_scrolling << ", Init Screen width = " << settings.init_screen_width << " icon offset " << icon_offset << std::endl;
};
void ScrollingLine::updateText(std::string *newLineString)
{
    currentLine = *newLineString;
};
void ScrollingLine::renderLine(FrameCanvas *offscreen_canvas)
{
    if (length <= max_width_for_no_scrolling)
    {
        speed = 0;
        x = ((max_width_for_no_scrolling - length) / 2) + icon_offset;
    }
    else if (speed == 0)
    {
        speed = orig_speed;
    }

    length = rgb_matrix::DrawText(offscreen_canvas, font,
                                  x, y + font.baseline(),
                                  color, nullptr,
                                  currentLine.c_str(), letter_spacing);

    if (speed > 0 && --x + length < icon_offset)
    {
        x = screen_width + 1;
    }
};