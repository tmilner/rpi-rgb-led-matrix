#ifndef UPDATEABLE_SCREEN_HPP
#define UPDATEABLE_SCREEN_HPP

#include "core/led-matrix.h"
#include "screens/screen.h"

class UpdateableScreen : public Screen {
public:
  UpdateableScreen(){};
  virtual ~UpdateableScreen(){};
  virtual void update() = 0;
  virtual void render(rgb_matrix::FrameCanvas *offscreen_canvas,
                      char opacity = 0xFF) = 0;
  virtual std::string *getName() = 0;

protected:
  std::string name;
};

#endif /*UPDATEABLE_SCREEN_HPP*/
