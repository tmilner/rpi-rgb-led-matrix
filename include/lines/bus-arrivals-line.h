#ifndef BUS_ARRIVALS_LINE_H
#define BUS_ARRIVALS_LINE_H
#include "clients/tfl-client.h"
#include "lines/scrolling-line.h"
#include "screens/updatable-screen.h"
#include <Magick++.h>
#include <chrono>

class BusArrivalsLine : public UpdatableScreen, public ScrollingLine {
public:
  BusArrivalsLine(
      std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
      TflClient *tflClient, ScrollingLineSettings settings);
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  Magick::Image *getIcon();
  TflClient *tflClient;
  std::chrono::time_point<std::chrono::system_clock> last_update;
  static const int update_after_seconds = 45;
  std::shared_ptr<std::map<std::string, Magick::Image>> image_map{};
  std::string image_key;
  std::string name;
};
#endif /*BUS_ARRIVALS_LINE_H*/
