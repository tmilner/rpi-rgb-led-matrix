#ifndef DATE_LINE_UPDATER_H
#define DATE_LINE_UPDATER_H
#include "led-matrix.h"
#include "scrolling-line.h"
#include "updateable-screen.h"
#include <Magick++.h>

class DateLine : public UpdateableScreen, public ScrollingLine {
public:
  DateLine(std::map<std::string, Magick::Image> *image_map,
           ScrollingLineSettings settings);
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  Magick::Image *getIcon();
  std::string date_image;
  std::map<std::string, Magick::Image> *image_map{};
  std::string name;
};

#endif /*DATE_LINE_UPDATER_H*/
