#include "lines/date-line.h"
#include "core/date.h"
#include "core/img_utils.h"
#include <iostream>
#include <mutex>

using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

DateLine::DateLine(
    std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
    ScrollingLineSettings settings)
    : ScrollingLine(settings), name{std::string("Date Line")} {
  std::cout << "Date Line Updater Constructor" << std::endl;

  this->current_line = "Loading";
  this->date_image = "date";
  this->image_map = image_map;
  this->is_visible = true;
  this->name = std::string("Date Line");
  std::cout << "Date Line Updater Constructor END" << std::endl;
}

std::string *DateLine::getName() { return &this->name; }

Magick::Image *DateLine::getIcon() {
  return &(*this->image_map)[this->date_image];
}

void DateLine::render(FrameCanvas *offscreen_canvas, char opacity) {
  if (opacity >= (CHAR_MAX / 2)) {
    this->renderLine(offscreen_canvas);
  }
  offscreen_canvas->SetPixels(0, this->y, 13, 16, 0, 0, 0);
  rgb_matrix::DrawLine(offscreen_canvas, 13, this->y, 13, this->y + 16,
                       Color(130, 100, 73));
  CopyImageToCanvas(this->getIcon(), offscreen_canvas, 0, this->y + 1, opacity);
}
void DateLine::update() {
  if (!is_visible) {
    return;
  }

  const auto now = std::chrono::system_clock::now();
  std::string date_text =
      date::format("%d %b", date::floor<std::chrono::milliseconds>(now));
  updateText(&date_text);
}
