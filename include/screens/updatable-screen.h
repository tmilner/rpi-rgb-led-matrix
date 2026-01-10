#ifndef UPDATABLE_SCREEN_HPP
#define UPDATABLE_SCREEN_HPP

#include "core/led-matrix.h"
#include "screens/screen.h"

class UpdatableScreen : public Screen {
public:
  UpdatableScreen(){};
  virtual ~UpdatableScreen(){};
  virtual void update() = 0;
  virtual void render(rgb_matrix::FrameCanvas *offscreen_canvas,
                      char opacity = 0xFF) = 0;
  virtual std::string *getName() = 0;

protected:
  std::string name;
};

#endif /*UPDATABLE_SCREEN_HPP*/
