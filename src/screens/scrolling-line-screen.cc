#include "screens/scrolling-line-screen.h"
#include "core/date.h"
#include "lines/line-registry.h"
#include <climits>
#include <iostream>

ScrollingLineScreen::ScrollingLineScreen(
    std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
    std::map<std::string, std::string> weather_icon_map,
    ScrollingLineScreenSettings settings, ServiceRegistry *service_registry)
    : image_map{image_map}, line1_settings{settings.speed,
                                           settings.speed_mutex,
                                           0,
                                           settings.letter_spacing,
                                           settings.font,
                                           settings.color,
                                           settings.width,
                                           14},
      line2_settings{settings.speed,      settings.speed_mutex,
                     settings.height / 2, settings.letter_spacing,
                     settings.font,       settings.color,
                     settings.width,      14},
      settings{settings}, bg_color{settings.bg_color},
      name{std::string("Scrolling Screen")}, service_registry(service_registry),
      line1_options{settings.line1_options},
      line2_options{settings.line2_options},
      line1_rotate_after_seconds{settings.line1_rotate_after_seconds},
      line2_rotate_after_seconds{settings.line2_rotate_after_seconds}

{
  this->is_visible = true;
  auto now = std::chrono::system_clock::now();
  this->line1_last_rotate = now;
  this->line2_last_rotate = now;

  this->weather_icon_map = std::move(weather_icon_map);
  this->line_context.image_map = this->image_map;
  this->line_context.weather_icon_map = this->weather_icon_map;
  this->line_context.weather_api_key = this->settings.weather_api_key;
  this->line_context.services = this->service_registry;

  this->line1_lines = BuildLineMap(this->line1_options, this->line1_settings,
                                   this->line_context);
  this->line2_lines = BuildLineMap(this->line2_options, this->line2_settings,
                                   this->line_context);

  this->line1 = this->line1_options.empty()
                    ? nullptr
                    : GetLine(this->line1_lines, this->line1_options.front());
  this->line2 = this->line2_options.empty()
                    ? nullptr
                    : GetLine(this->line2_lines, this->line2_options.front());
  this->line1_transitioning = false;
  this->line1_transition_percentage = CHAR_MAX;
  this->previous_line1 = this->line1;
  this->line2_transitioning = false;
  this->line2_transition_percentage = CHAR_MAX;
  this->previous_line2 = this->line2;

  if (!this->line1_options.empty() && this->line1 != nullptr) {
    this->setLine1(this->line1_options.front());
  }
  if (!this->line2_options.empty() && this->line2 != nullptr) {
    this->setLine2(this->line2_options.front());
  }
}

std::string *ScrollingLineScreen::getName() { return &this->name; }

void ScrollingLineScreen::render(FrameCanvas *offscreen_canvas, char opacity) {
  if (!is_visible) {
    return;
  }

  std::lock_guard<std::mutex> lock(transition_mutex);
  char rate = CHAR_MAX / 10;

  offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
  if (this->line1 != nullptr && this->line1_transitioning &&
      this->previous_line1 != nullptr) {
    if (this->line1_transition_percentage < (CHAR_MAX / 2)) {
      this->previous_line1->render(
          offscreen_canvas, CHAR_MAX - this->line1_transition_percentage);
    } else {
      this->line1->render(offscreen_canvas, this->line1_transition_percentage);
    }
    this->line1_transition_percentage += rate;
    if (this->line1_transition_percentage >= CHAR_MAX - rate) {
      this->line1_transitioning = false;
    }
  } else if (this->line1 != nullptr) {
    this->line1->render(offscreen_canvas, CHAR_MAX);
  }

  if (this->line2 != nullptr && this->line2_transitioning &&
      this->previous_line2 != nullptr) {
    if (this->line2_transition_percentage < (CHAR_MAX / 2)) {
      this->previous_line2->render(
          offscreen_canvas, CHAR_MAX - this->line2_transition_percentage);
    } else {
      this->line2->render(offscreen_canvas, this->line2_transition_percentage);
    }
    this->line2_transition_percentage += rate;
    if (this->line2_transition_percentage >= CHAR_MAX - rate) {
      this->line2_transitioning = false;
    }
  } else if (this->line2 != nullptr) {
    this->line2->render(offscreen_canvas, CHAR_MAX);
  }
}

void ScrollingLineScreen::setLine1(LineType type) {
  std::lock_guard<std::mutex> lock(transition_mutex);
  this->line1_transitioning = true;
  this->previous_line1 = this->line1;
  this->line1_transition_percentage = 0x00;

  UpdatableScreen *next_line = GetLine(this->line1_lines, type);
  if (next_line == nullptr) {
    return;
  }
  ResetLinePosition(next_line);
  this->line1 = next_line;
  if (this->line1_options.size() <= 1) {
    this->line1_transitioning = false;
    this->line1_transition_percentage = CHAR_MAX;
  }
  this->current_line1 = type;
}
void ScrollingLineScreen::setLine2(LineType type) {
  std::lock_guard<std::mutex> lock(transition_mutex);
  this->line2_transitioning = true;
  this->previous_line2 = this->line2;
  this->line2_transition_percentage = 0x00;

  UpdatableScreen *next_line = GetLine(this->line2_lines, type);
  if (next_line == nullptr) {
    return;
  }
  ResetLinePosition(next_line);
  this->line2 = next_line;
  if (this->line2_options.size() <= 1) {
    this->line2_transitioning = false;
    this->line2_transition_percentage = CHAR_MAX;
  }
  this->current_line2 = type;
}

void ScrollingLineScreen::setLine1Options(std::vector<LineType> options) {
  LineMap new_lines =
      BuildLineMap(options, this->line1_settings, this->line_context);
  LineType next_line = LineType::Bus;
  bool should_update = false;
  {
    std::lock_guard<std::mutex> lock(transition_mutex);
    this->line1_options = std::move(options);
    this->line1_lines = std::move(new_lines);
    this->line1_index = 0;
    if (!this->line1_options.empty()) {
      next_line = this->line1_options.front();
      should_update = true;
    }
  }
  if (should_update) {
    this->setLine1(next_line);
  }
}

void ScrollingLineScreen::setLine2Options(std::vector<LineType> options) {
  LineMap new_lines =
      BuildLineMap(options, this->line2_settings, this->line_context);
  LineType next_line = LineType::CurrentTime;
  bool should_update = false;
  {
    std::lock_guard<std::mutex> lock(transition_mutex);
    this->line2_options = std::move(options);
    this->line2_lines = std::move(new_lines);
    this->line2_index = 0;
    if (!this->line2_options.empty()) {
      next_line = this->line2_options.front();
      should_update = true;
    }
  }
  if (should_update) {
    this->setLine2(next_line);
  }
}

void ScrollingLineScreen::update() {
  const auto now = std::chrono::system_clock::now();

  if (!this->line1_options.empty() &&
      (now - this->line1_last_rotate) > this->line1_rotate_after_seconds) {
    std::cout << "Changing line 1" << this->line1->getName()
              << " Last rotate is "
              << date::format("%D %T", date::floor<std::chrono::milliseconds>(
                                           this->line1_last_rotate))
              << std::endl;

    this->line1_last_rotate = now;
    this->line1_index = (this->line1_index + 1) % this->line1_options.size();
    this->setLine1(this->line1_options[this->line1_index]);
  }

  if (!this->line2_options.empty() &&
      (now - this->line2_last_rotate) > this->line2_rotate_after_seconds) {
    std::cout << "Changing line 2" << this->line2->getName()
              << " Last rotate is "
              << date::format("%D %T", date::floor<std::chrono::milliseconds>(
                                           this->line2_last_rotate))
              << std::endl;

    this->line2_last_rotate = now;
    this->line2_index = (this->line2_index + 1) % this->line2_options.size();
    this->setLine2(this->line2_options[this->line2_index]);
  }

  if (this->line1 != nullptr) {
    this->line1->update();
  }
  if (this->line2 != nullptr) {
    this->line2->update();
  }
}
