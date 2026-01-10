#ifndef SCROLLING_SCREEN_H
#define SCROLLING_SCREEN_H
#include "core/service-registry.h"
#include "lines/line-registry.h"
#include "lines/scrolling-line.h"
#include "screens/updatable-screen.h"
#include <Magick++.h>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

struct ScrollingLineScreenSettings : ScreenSettings {
  int width;
  int height;
  rgb_matrix::Font *font;
  rgb_matrix::Color color;
  rgb_matrix::Color bg_color;
  float *speed;
  std::mutex *speed_mutex;
  int letter_spacing;
  std::vector<LineType> line1_options;
  std::vector<LineType> line2_options;
  std::chrono::seconds line1_rotate_after_seconds;
  std::chrono::seconds line2_rotate_after_seconds;
  std::string weather_api_key;
  ScrollingLineScreenSettings(int width, int height, rgb_matrix::Font *font,
                              rgb_matrix::Color color,
                              rgb_matrix::Color bg_color, float *speed,
                              std::mutex *speed_mutex,
                              std::vector<LineType> line1_options,
                              std::vector<LineType> line2_options,
                              std::chrono::seconds line1_rotate_after_seconds,
                              std::chrono::seconds line2_rotate_after_seconds,
                              int letter_spacing,
                              const std::string weather_api_key) {
    this->width = width;
    this->height = height;
    this->font = font;
    this->color = color;
    this->bg_color = bg_color;
    this->speed = speed;
    this->speed_mutex = speed_mutex;
    this->line1_options = std::move(line1_options);
    this->line2_options = std::move(line2_options);
    this->line1_rotate_after_seconds = line1_rotate_after_seconds;
    this->line2_rotate_after_seconds = line2_rotate_after_seconds;
    this->weather_api_key = weather_api_key;
    this->letter_spacing = letter_spacing;
  }
};

class ScrollingLineScreen : public UpdatableScreen {
public:
  ScrollingLineScreen(
      std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
      std::map<std::string, std::string> weather_icon_map,
      ScrollingLineScreenSettings settings, ServiceRegistry *service_registry);
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  void setLine1(LineType type);
  void setLine2(LineType type);
  void setLine1Options(std::vector<LineType> options);
  void setLine2Options(std::vector<LineType> options);
  std::string *getName();

private:
  mutable std::mutex transition_mutex;
  ScrollingLineScreenSettings settings;
  ServiceRegistry *service_registry;
  rgb_matrix::Color bg_color;
  std::string name;
  std::shared_ptr<std::map<std::string, Magick::Image>> image_map;
  std::map<std::string, std::string> weather_icon_map;
  LineRegistryContext line_context;
  LineMap line1_lines;
  LineMap line2_lines;

  UpdatableScreen *line1;
  UpdatableScreen *line2;
  ScrollingLineSettings line1_settings;
  ScrollingLineSettings line2_settings;

  std::vector<LineType> line1_options;
  std::vector<LineType> line2_options;
  size_t line1_index = 0;
  size_t line2_index = 0;

  LineType current_line1;
  UpdatableScreen *previous_line1;
  char line1_transition_percentage;
  bool line1_transitioning;
  std::chrono::time_point<std::chrono::system_clock> line1_last_rotate;
  std::chrono::seconds line1_rotate_after_seconds;

  LineType current_line2;
  UpdatableScreen *previous_line2;
  char line2_transition_percentage;
  bool line2_transitioning;
  std::chrono::time_point<std::chrono::system_clock> line2_last_rotate;
  std::chrono::seconds line2_rotate_after_seconds;
};
#endif /*SCROLLING_SCREEN_H*/
