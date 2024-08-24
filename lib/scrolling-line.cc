#include "scrolling-line.h"
#include "graphics.h"
#include "led-matrix.h"

#include <iostream>
#include <string>
using namespace rgb_matrix;

// ScrollingLine::ScrollingLine()
// {
//     x = 0;
//     y = 0;
//     length = 0;
//     letter_spacing = 0;

//     orig_speed = 0;
//     speed = 0;
//     icon_offset = 0;
//     screen_width = 0;
//     max_width_for_no_scrolling = 0;
// }
ScrollingLine::ScrollingLine(ScrollingLineSettings settings) {
  current_line = "Loading";
  x = settings.init_icon_offset;
  y = settings.init_y;
  length = 0;
  letter_spacing = settings.init_letter_spacing;
  color = settings.init_color;
  font = *settings.init_font;
  orig_speed = settings.init_speed;
  speed = *settings.init_speed;
  icon_offset = settings.init_icon_offset;
  screen_width = settings.init_screen_width;
  max_width_for_no_scrolling = (settings.init_screen_width - icon_offset);

  if (speed == 0) {
    x = 14;
  }

  std::cout << "Line created! Max width for scrolling = "
            << max_width_for_no_scrolling
            << ", Init Screen width = " << settings.init_screen_width
            << " icon offset " << icon_offset << std::endl;
};
void ScrollingLine::resetXPosition() { x = icon_offset + 20; }
void ScrollingLine::updateText(std::string *new_line_string) {
  if (*new_line_string != current_line) {
    x = this->icon_offset;
  }
  current_line = *new_line_string;
};
void ScrollingLine::renderLine(FrameCanvas *offscreen_canvas) {
  if (length <= max_width_for_no_scrolling) {
    speed = 0;
    x = ((max_width_for_no_scrolling - length) / 2) + icon_offset;
  } else {
    speed = *orig_speed;
  }

  length = rgb_matrix::DrawText(offscreen_canvas, font, x,
                                y + font.baseline() + 1, color, nullptr,
                                current_line.c_str(), letter_spacing);

  if (speed > 0 && --x + length < icon_offset) {
    x = screen_width + 1;
  }
};

void ScrollingLine::changeYPos(int new_y) { this->y = new_y; }
