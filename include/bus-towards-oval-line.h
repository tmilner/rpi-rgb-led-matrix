#ifndef BUS_TOWARDS_OVAL_LINE_H
#define BUS_TOWARDS_OVAL_LINE_H
#include "scrolling-line.h"
#include "tfl-client.h"
#include "updateable-screen.h"
#include <Magick++.h>
#include <chrono>

class BusTowardsOvalLine : public UpdateableScreen, public ScrollingLine {
public:
  BusTowardsOvalLine(
      std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
      TflClient *tflClient, ScrollingLineSettings settings);
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  Magick::Image *getIcon();
  TflClient *tflClient;
  std::chrono::time_point<std::chrono::system_clock> last_update;
  static const int update_after_seconds = 25;
  std::shared_ptr<std::map<std::string, Magick::Image>> image_map{};
  std::string image_key;
  std::string name;
};
#endif /*BUS_TOWARDS_OVAL_LINE_H*/
