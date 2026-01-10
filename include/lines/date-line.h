#ifndef DATE_LINE_UPDATER_H
#define DATE_LINE_UPDATER_H
#include "core/led-matrix.h"
#include "lines/scrolling-line.h"
#include "screens/updateable-screen.h"
#include <Magick++.h>
#include <memory>

class DateLine : public UpdateableScreen, public ScrollingLine {
public:
  DateLine(std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
           ScrollingLineSettings settings);
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  Magick::Image *getIcon();
  std::string date_image;
  std::shared_ptr<std::map<std::string, Magick::Image>> image_map{};
  std::string name;
};

#endif /*DATE_LINE_UPDATER_H*/
