#include "current-time-line.h"
#include "img_utils.h"
#include <chrono>
#include <climits>
#include <iomanip>
#include <iostream>

using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

CurrentTimeLine::CurrentTimeLine(
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

std::string *CurrentTimeLine::getName() { return &this->name; }

Magick::Image *CurrentTimeLine::getIcon() {
  return &(*this->image_map)[this->time_image];
}

void CurrentTimeLine::render(FrameCanvas *offscreen_canvas, char opacity) {
  if (!is_visible) {
    return;
  }
  if (opacity >= (CHAR_MAX / 2)) {
    this->renderLine(offscreen_canvas);
  }
  offscreen_canvas->SetPixels(0, this->y, 13, 16, 0, 0, 0);
  rgb_matrix::DrawLine(offscreen_canvas, 13, this->y, 13, this->y + 16,
                       Color(130, 100, 73));
  CopyImageToCanvas(this->getIcon(), offscreen_canvas, 0, this->y + 1, opacity);
}
void CurrentTimeLine::update() {
  if (!is_visible) {
    return;
  }

  const auto now = std::chrono::system_clock::now();
  this->current_line.clear();
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%H:%M");
  auto time = oss.str();
  this->current_line.append(time);
}
