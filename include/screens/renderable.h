#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include "core/led-matrix.h"

class Renderable {
public:
  virtual void render(rgb_matrix::FrameCanvas *offscreen_canvas,
                      char opacity = 0xFF) = 0;
};

#endif /*RENDERABLE_HPP*/
