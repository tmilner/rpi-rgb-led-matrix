#ifndef TIME_LINE_H
#define TIME_LINE_H
#include "core/led-matrix.h"
#include "lines/scrolling-line.h"
#include "screens/updatable-screen.h"
#include <Magick++.h>
#include <memory>

class TimeLine : public UpdatableScreen, public ScrollingLine {
public:
  TimeLine(
      std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
      ScrollingLineSettings settings);
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  Magick::Image *getIcon();
  std::string time_image;
  std::shared_ptr<std::map<std::string, Magick::Image>> image_map{};
  std::string name;
};

#endif /*TIME_LINE_H*/
