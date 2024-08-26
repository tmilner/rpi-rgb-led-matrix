#ifndef ROTATING_BOX_H
#define ROTATING_BOX_H

#include "screen.h"

class RotatingBox : public Screen {
public:
  RotatingBox(rgb_matrix::FrameCanvas *offscreen_canvas);
  void render(rgb_matrix::FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  std::string name;
  const int cent_x;
  const int cent_y;
  const int rotate_square;
  const int min_rotate;
  const int max_rotate;
  const int display_square;
  const int min_display;
  const int max_display;
  const float deg_to_rad = 2 * 3.14159265 / 360;
  int rotation;
  void Rotate(int x, int y, float angle, float *new_x, float *new_y);
  uint8_t scale_col(int val, int lo, int hi);
};

#endif /*ROTATING_BOX_H*/
