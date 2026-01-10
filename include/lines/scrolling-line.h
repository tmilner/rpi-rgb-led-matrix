#ifndef SCROLLING_LINE_H
#define SCROLLING_LINE_H
#include "core/graphics.h"
#include "core/led-matrix.h"
#include <mutex>
#include <string>

using namespace rgb_matrix;

struct ScrollingLineSettings {
  float *init_speed;
  std::mutex *init_speed_mutex;
  int init_y;
  int init_letter_spacing;
  Font *init_font;
  Color init_color;
  int init_screen_width;
  int init_icon_offset;
  ScrollingLineSettings(float *speed, std::mutex *speed_mutex, int y,
                        int letter_space, Font *font, Color color,
                        int screen_width, int icon_offset) {
    this->init_speed = speed;
    this->init_speed_mutex = speed_mutex;
    this->init_y = y;
    this->init_letter_spacing = letter_space;
    this->init_font = font;
    this->init_color = color;
    this->init_screen_width = screen_width;
    this->init_icon_offset = icon_offset;
  }
};

class ScrollingLine {
protected:
  std::string current_line;
  std::string pending_line;
  bool has_pending_line = false;
  int x;
  int y;
  int length;
  int letter_spacing;
  float speed;
  float *orig_speed;
  int icon_offset;
  int screen_width;
  int max_width_for_no_scrolling;
  rgb_matrix::Font font;
  Color color;
  mutable std::recursive_mutex line_mutex;
  std::mutex *speed_mutex;
  bool isReadyForUpdateLocked() const;

public:
  ScrollingLine(ScrollingLineSettings settings);
  void updateText(std::string *new_line_string);
  void updateTextImmediate(std::string *new_line_string);
  void renderLine(FrameCanvas *offscreen_canvas);
  void resetXPosition();
  void changeYPos(int new_y);
  bool isReadyForUpdate();
};
#endif /*SCROLLING_LINE_H*/
