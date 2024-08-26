#include "scrolling-line-screen.h"
#include "bus-towards-oval-line.h"
#include "date.h"
#include "img_utils.h"
#include "music-line.h"
#include "time-date-weather-line.h"
#include <climits>
#include <iostream>

using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

ScrollingLineScreen::ScrollingLineScreen(
    std::map<std::string, Magick::Image> *image_map,
    ScrollingLineScreenSettings settings, SpotifyClient spotify_client,
    Radio6Client radio6_client, TflClient tfl_client)
    : image_map{image_map},
      line1_settings{settings.speed, 0, 0, settings.font, settings.color,
                     settings.width, 14},
      line2_settings{settings.speed, settings.height / 2, 0, settings.font,
                     settings.color, settings.width,      14},
      settings{settings}, bg_color{settings.bg_color},
      name{std::string("Scrolling Screen")}, spotify_client(spotify_client),
      radio6_client(radio6_client), tfl_client(tfl_client)

{
  this->image_map = image_map;
  this->is_visible = true;
  auto now = std::chrono::system_clock::now();
  this->last_rotate = now;

  this->music_line = new MusicLine(this->image_map, this->spotify_client,
                                   this->radio6_client, this->line1_settings);

  this->bus_line = new BusTowardsOvalLine(this->image_map, this->tfl_client,
                                          this->line1_settings);
  this->setLine1(this->settings.line1);
  this->setLine2(this->settings.line2);
  this->line1_transitioning = false;
  this->line1_transition_percentage = CHAR_MAX;
  this->previous_line1 = this->bus_line;
}

std::string *ScrollingLineScreen::getName() { return &this->name; }

void ScrollingLineScreen::render(FrameCanvas *offscreen_canvas, char opacity) {
  if (!is_visible) {
    return;
  }

  offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
  if (this->line1_transitioning) {
    this->previous_line1->render(offscreen_canvas,
                                 CHAR_MAX - this->line1_transition_percentage);
    this->line1->render(offscreen_canvas, this->line1_transition_percentage);
    if (this->line1_transition_percentage >= CHAR_MAX - 10) {
      this->line1_transitioning = false;
    }
    this->line1_transition_percentage += 50;
  } else {
    this->line1->render(offscreen_canvas, CHAR_MAX);
  }
  this->line2->render(offscreen_canvas);
}

void ScrollingLineScreen::setLine1(ScreenLineOption type) {
  this->line1_transitioning = true;
  this->previous_line1 = this->line1;
  this->line1_transition_percentage = 0x00;

  if (type == ScreenLineOption::radio6) {
    this->music_line->resetXPosition();
    this->line1 = this->music_line;
  } else if (type == ScreenLineOption::bus) {
    this->bus_line->resetXPosition();
    this->line1 = this->bus_line;
  } else {
    TimeDateWeatherLine *timeDateWeatherLine = new TimeDateWeatherLine(
        this->settings.weather_api_key, this->image_map, this->line1_settings);
    delete this->line1;
    this->line1 = timeDateWeatherLine;
  }
  this->current_line1 = type;
}
void ScrollingLineScreen::setLine2(ScreenLineOption type) {
  if (type == ScreenLineOption::radio6) {
    MusicLine *musicLine =
        new MusicLine(this->image_map, this->spotify_client,
                      this->radio6_client, this->line2_settings);
    delete this->line2;
    this->line2 = musicLine;
  } else if (type == ScreenLineOption::bus) {
    BusTowardsOvalLine *busTowardsOvalLine = new BusTowardsOvalLine(
        this->image_map, this->tfl_client, this->line2_settings);
    delete this->line2;
    this->line2 = busTowardsOvalLine;
  } else {
    TimeDateWeatherLine *timeDateWeatherLine = new TimeDateWeatherLine(
        this->settings.weather_api_key, this->image_map, this->line2_settings);
    delete this->line2;
    this->line2 = timeDateWeatherLine;
  }
}
void ScrollingLineScreen::update() {
  if (!is_visible) {
    return;
  }
  const auto now = std::chrono::system_clock::now();
  bool screen_rotated = false;

  if (((now - this->last_rotate) / 1s) > this->rotate_after_seconds) {
    screen_rotated = true;
    std::cout << "Changing line 1" << this->line1->getName()
              << " Last rotate is "
              << date::format("%D %T", date::floor<std::chrono::milliseconds>(
                                           this->last_rotate))
              << std::endl;

    this->last_rotate = now;

    if (this->current_line1 == ScreenLineOption::bus) {
      this->setLine1(ScreenLineOption::radio6);
    } else
      this->setLine1(ScreenLineOption::bus);
  }

  this->music_line->update();
  this->bus_line->update();
  this->line2->update();
}
