#ifndef TIME_LINE_UPDATER_H
#define TIME_LINE_UPDATER_H
#include "led-matrix.h"
#include "scrolling-line.h"
#include "updateable-screen.h"
#include <Magick++.h>
#include <memory>

class CurrentTimeLine : public UpdateableScreen, public ScrollingLine {
public:
  CurrentTimeLine(
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

#endif /*TIME_LINE_UPDATER_H*/
