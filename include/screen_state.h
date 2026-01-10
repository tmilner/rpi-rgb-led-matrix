#ifndef SCREEN_STATE_H
#define SCREEN_STATE_H

#include <map>
#include <mutex>
#include <string>

#include <Magick++.h>

#include "screen_mode.h"

namespace rgb_matrix {
struct ScreenState {
  std::map<std::string, Magick::Image> image_map{};
  ScreenMode current_mode = ScreenMode::scrolling_lines;
  int current_brightness = 50;
  bool screen_on = true;
  float speed = 2.0f;
  mutable std::mutex mutex;
};
} // namespace rgb_matrix
#endif /*SCREEN_STATE_H*/
