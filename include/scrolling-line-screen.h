#ifndef SCROLLING_SCREEN_H
#define SCROLLING_SCREEN_H
#include "bus-towards-oval-line.h"
#include "current-time-line.h"
#include "date-line.h"
#include "music-line.h"
#include "radio6-client.h"
#include "scrolling-line.h"
#include "spotify-client.h"
#include "tfl-client.h"
#include "updateable-screen.h"
#include "weather-line.h"
#include <Magick++.h>

enum ScreenLineOption {
  radio6,
  current_time,
  current_date,
  weather,
  timeDateWeather,
  bus
};

struct ScrollingLineScreenSettings : ScreenSettings {
  int width;
  int height;
  rgb_matrix::Font *font;
  rgb_matrix::Color color;
  rgb_matrix::Color bg_color;
  float *speed;
  int letter_spacing;
  ScreenLineOption line1;
  ScreenLineOption line2;
  std::string weather_api_key;
  ScrollingLineScreenSettings(int width, int height, rgb_matrix::Font *font,
                              rgb_matrix::Color color,
                              rgb_matrix::Color bg_color, float *speed,
                              int letter_spacing, ScreenLineOption line1,
                              ScreenLineOption line2,
                              const std::string weather_api_key) {
    this->width = width;
    this->height = height;
    this->font = font;
    this->color = color;
    this->bg_color = bg_color;
    this->speed = speed;
    this->line1 = line1;
    this->line2 = line2;
    this->weather_api_key = weather_api_key;
  }
};

class ScrollingLineScreen : public UpdateableScreen {
public:
  ScrollingLineScreen(
      std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
      std::map<std::string, std::string> weather_icon_map,
      ScrollingLineScreenSettings settings, SpotifyClient spotify_client,
      Radio6Client radio6_client, TflClient tfl_client);
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  void setLine1(ScreenLineOption type);
  void setLine2(ScreenLineOption type);
  std::string *getName();

private:
  ScrollingLineScreenSettings settings;
  SpotifyClient spotify_client;
  Radio6Client radio6_client;
  TflClient tfl_client;
  rgb_matrix::Color bg_color;
  std::string name;
  std::shared_ptr<std::map<std::string, Magick::Image>> image_map;
  BusTowardsOvalLine *bus_line;
  MusicLine *music_line;
  CurrentTimeLine *time_line;
  DateLine *date_line;
  WeatherLine *weather_line;

  UpdateableScreen *line1;
  UpdateableScreen *line2;
  ScrollingLineSettings line1_settings;
  ScrollingLineSettings line2_settings;

  ScreenLineOption current_line1;
  UpdateableScreen *previous_line1;
  char line1_transition_percentage;
  bool line1_transitioning;
  std::chrono::time_point<std::chrono::system_clock> line1_last_rotate;
  static const int line1_rotate_after_seconds = 15;

  ScreenLineOption current_line2;
  UpdateableScreen *previous_line2;
  char line2_transition_percentage;
  bool line2_transitioning;
  std::chrono::time_point<std::chrono::system_clock> line2_last_rotate;
  static const int line2_rotate_after_seconds = 10;
};
#endif /*SCROLLING_SCREEN_H*/
