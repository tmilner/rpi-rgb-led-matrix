#include "lines/time-line.h"
#include "core/img_utils.h"
#include <chrono>
#include <climits>
#include <iomanip>
#include <iostream>
#include <mutex>

using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

TimeLine::TimeLine(
    std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
    ScrollingLineSettings settings)
    : ScrollingLine(settings), name{std::string("Time Line")} {
  std::cout << "Time Line Updater Constructor" << std::endl;

  this->current_line = "Loading";
  this->time_image = "time";
  auto now = std::chrono::system_clock::now();
  this->image_map = image_map;
  this->is_visible = true;
  this->name = std::string("Time Line");
  std::cout << "Time Line Updater Constructor END" << std::endl;
}

std::string *TimeLine::getName() { return &this->name; }

Magick::Image *TimeLine::getIcon() {
  return &(*this->image_map)[this->time_image];
}

void TimeLine::render(FrameCanvas *offscreen_canvas, char opacity) {
  if (opacity >= (CHAR_MAX / 2)) {
    this->renderLine(offscreen_canvas);
  }
  offscreen_canvas->SetPixels(0, this->y, 13, 16, 0, 0, 0);
  rgb_matrix::DrawLine(offscreen_canvas, 13, this->y, 13, this->y + 16,
                       Color(130, 100, 73));
  CopyImageToCanvas(this->getIcon(), offscreen_canvas, 0, this->y + 1, opacity);
}
void TimeLine::update() {
  if (!is_visible) {
    return;
  }

  const auto now = std::chrono::system_clock::now();
  std::lock_guard<std::recursive_mutex> lock(line_mutex);
  this->current_line.clear();
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%H:%M");
  auto time = oss.str();
  this->current_line.append(time);
}
