#include "screens/scrolling-line-screen.h"
#include "lines/bus-arrivals-line.h"
#include "lines/time-line.h"
#include "core/date.h"
#include "core/img_utils.h"
#include "lines/music-info-line.h"
#include "lines/weather-line.h"
#include <climits>
#include <iostream>

ScrollingLineScreen::ScrollingLineScreen(
    std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
    std::map<std::string, std::string> weather_icon_map,
    ScrollingLineScreenSettings settings, SpotifyClient *spotify_client,
    Radio6Client *radio6_client, TflClient *tfl_client)
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
      name{std::string("Scrolling Screen")}, spotify_client(spotify_client),
      radio6_client(radio6_client), tfl_client(tfl_client),
      line1_options{settings.line1_options},
      line2_options{settings.line2_options},
      line1_rotate_after_seconds{settings.line1_rotate_after_seconds},
      line2_rotate_after_seconds{settings.line2_rotate_after_seconds}

{
  this->is_visible = true;
  auto now = std::chrono::system_clock::now();
  this->line1_last_rotate = now;
  this->line2_last_rotate = now;

  this->music_line =
      std::make_unique<MusicInfoLine>(this->image_map, this->spotify_client,
                                      this->radio6_client,
                                      this->line1_settings);

  this->bus_line = std::make_unique<BusArrivalsLine>(
      this->image_map, this->tfl_client, this->line1_settings);

  this->time_line =
      std::make_unique<TimeLine>(this->image_map, this->line2_settings);
  this->date_line =
      std::make_unique<DateLine>(this->image_map, this->line2_settings);
  this->weather_line = std::make_unique<WeatherLine>(
      this->settings.weather_api_key, weather_icon_map, this->image_map,
      this->line2_settings);

  this->line1 = this->bus_line.get();
  this->line2 = this->date_line.get();

  if (!this->line1_options.empty()) {
    this->setLine1(this->line1_options.front());
  }
  if (!this->line2_options.empty()) {
    this->setLine2(this->line2_options.front());
  }
  this->line1_transitioning = false;
  this->line1_transition_percentage = CHAR_MAX;
  this->previous_line1 = this->bus_line.get();
  this->line2_transitioning = false;
  this->line2_transition_percentage = CHAR_MAX;
  this->previous_line2 = this->date_line.get();
}

std::string *ScrollingLineScreen::getName() { return &this->name; }

void ScrollingLineScreen::render(FrameCanvas *offscreen_canvas, char opacity) {
  if (!is_visible) {
    return;
  }

  std::lock_guard<std::mutex> lock(transition_mutex);
  char rate = CHAR_MAX / 10;

  offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
  if (this->line1_transitioning) {
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
  } else {
    this->line1->render(offscreen_canvas, CHAR_MAX);
  }

  if (this->line2_transitioning) {
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
  } else {
    this->line2->render(offscreen_canvas, CHAR_MAX);
  }
}

void ScrollingLineScreen::setLine1(ScreenLineOption type) {
  std::lock_guard<std::mutex> lock(transition_mutex);
  this->line1_transitioning = true;
  this->previous_line1 = this->line1;
  this->line1_transition_percentage = 0x00;

  if (type == ScreenLineOption::radio6) {
    this->music_line->resetXPosition();
    this->line1 = this->music_line.get();
  } else if (type == ScreenLineOption::bus) {
    this->bus_line->resetXPosition();
    this->line1 = this->bus_line.get();
  }
  if (this->line1_options.size() <= 1) {
    this->line1_transitioning = false;
    this->line1_transition_percentage = CHAR_MAX;
  }
  this->current_line1 = type;
}
void ScrollingLineScreen::setLine2(ScreenLineOption type) {
  std::lock_guard<std::mutex> lock(transition_mutex);
  this->line2_transitioning = true;
  this->previous_line2 = this->line2;
  this->line2_transition_percentage = 0x00;

  if (type == ScreenLineOption::current_date) {
    this->date_line->resetXPosition();
    this->line2 = this->date_line.get();
  } else if (type == ScreenLineOption::current_time) {
    this->time_line->resetXPosition();
    this->line2 = this->time_line.get();
  } else {
    this->weather_line->resetXPosition();
    this->line2 = this->weather_line.get();
  }
  if (this->line2_options.size() <= 1) {
    this->line2_transitioning = false;
    this->line2_transition_percentage = CHAR_MAX;
  }
  this->current_line2 = type;
}

void ScrollingLineScreen::setLine1Options(
    std::vector<ScreenLineOption> options) {
  ScreenLineOption next_line = ScreenLineOption::bus;
  bool should_update = false;
  {
    std::lock_guard<std::mutex> lock(transition_mutex);
    this->line1_options = std::move(options);
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

void ScrollingLineScreen::setLine2Options(
    std::vector<ScreenLineOption> options) {
  ScreenLineOption next_line = ScreenLineOption::current_time;
  bool should_update = false;
  {
    std::lock_guard<std::mutex> lock(transition_mutex);
    this->line2_options = std::move(options);
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

  if (this->current_line1 == ScreenLineOption::radio6) {
    this->music_line->update();
  } else if (this->current_line1 == ScreenLineOption::bus) {
    this->bus_line->update();
  }

  if (this->current_line2 == ScreenLineOption::current_time) {
    this->time_line->update();
  } else if (this->current_line2 == ScreenLineOption::current_date) {
    this->date_line->update();
  } else if (this->current_line2 == ScreenLineOption::weather) {
    this->weather_line->update();
  }
}

