#ifndef WEATHER_LINE_UPDATER_H
#define WEATHER_LINE_UPDATER_H
#include "json-fetcher.h"
#include "led-matrix.h"
#include "scrolling-line.h"
#include "updateable-screen.h"
#include <Magick++.h>
#include <chrono>

class WeatherLine : public UpdateableScreen, public ScrollingLine {
public:
  WeatherLine(const std::string weather_api_key,
              std::map<std::string, std::string> weather_icon_map,
              std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
              ScrollingLineSettings settings);
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  Magick::Image *getIcon();
  Magick::Image *getBlankIcon();
  std::map<std::string, std::string> weather_icon_map;
  JSONFetcher *fetcher;
  std::string url;
  std::string weather_image;
  std::chrono::time_point<std::chrono::system_clock> last_weather_update;
  static const int update_weather_after_seconds = 360;
  std::shared_ptr<std::map<std::string, Magick::Image>> image_map{};
  std::string name;
};

#endif /*WEATHER_LINE_UPDATER_H*/
