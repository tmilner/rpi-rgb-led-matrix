#include "led-matrix.h"
#include "graphics.h"

#include <string>
#include <iostream>
#include "screen-line.h"

using namespace rgb_matrix;

ScreenLine::ScreenLine(int init_speed, int init_y, int init_letter_spacing, Font *init_font, Color init_color, std::string *starting_line, int init_screen_width, int init_icon_offset)
{
    currentLine = *starting_line;
    x = init_icon_offset;
    y = init_y;
    length = 0;
    letter_spacing = init_letter_spacing;
    color = init_color;
    font = *init_font;
    orig_speed = init_speed;
    speed = init_speed;
    icon_offset = init_icon_offset;
    screen_width = init_screen_width;
    max_width_for_no_scrolling = (init_screen_width - icon_offset);

    if(speed == 0) {
        x = 14;
    }

    std::cout << "Line created! Max width for scrolling = " << max_width_for_no_scrolling << ", Init Screen width = " << init_screen_width << " icon offset " << icon_offset << std::endl; 
};
void ScreenLine::updateText(std::string *newLineString)
{
    currentLine = *newLineString;
};
void ScreenLine::render(FrameCanvas *offscreen_canvas)
{
    if(length <= max_width_for_no_scrolling) {
        speed = 0;
        x = ((max_width_for_no_scrolling - length)/ 2 ) + icon_offset;
    } else if(speed == 0) {
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