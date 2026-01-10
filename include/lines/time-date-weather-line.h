#ifndef TIME_DATE_WEATHER_LINE_H
#define TIME_DATE_WEATHER_LINE_H
#include "clients/json-fetcher.h"
#include "core/led-matrix.h"
#include "lines/scrolling-line.h"
#include "screens/updatable-screen.h"
#include <Magick++.h>
#include <chrono>

class TimeDateWeatherLine : public UpdatableScreen, ScrollingLine {
public:
  TimeDateWeatherLine(const std::string weather_api_key,
                      std::map<std::string, Magick::Image> *image_map,
                      ScrollingLineSettings settings);
  ~TimeDateWeatherLine();
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  Magick::Image *getIcon();
  JSONFetcher *fetcher;
  std::string url;
  std::string current_image;
  std::string weather_line;
  std::string weather_image;
  std::string time_image;
  std::string date_image;
  std::chrono::time_point<std::chrono::system_clock> last_weather_update;
  std::chrono::time_point<std::chrono::system_clock> last_rotate;
  int current_display;
  static const int rotate_after_seconds = 15;
  static const int update_weather_after_seconds = 360;
  std::map<std::string, Magick::Image> *image_map{};
  std::string name;
};

#endif /*TIME_DATE_WEATHER_LINE_H*/
