#include "weather-line.h"
#include "img_utils.h"
#include <iomanip>
#include <iostream>

using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

WeatherLine::WeatherLine(const std::string weather_api_key,
                         std::map<std::string, Magick::Image> *image_map,
                         ScrollingLineSettings settings)
    : ScrollingLine(settings), name{std::string("Weather Line")} {
  std::cout << "Weather Line Updater Constructor" << std::endl;

  const std::string weather_base_url(
      "https://api.openweathermap.org/data/2.5/"
      "weather?lon=-0.093014&lat=51.474087&appid=");

  this->current_line = "Loading";
  this->weather_image = "01d";
  this->url = weather_base_url + weather_api_key;
  auto now = std::chrono::system_clock::now();
  this->last_weather_update = now - 5min;
  this->image_map = image_map;
  this->is_visible = true;
  this->fetcher = new JSONFetcher();
  this->name = std::string("Weather Line");
  std::cout << "Weather Line Updater Constructor END" << std::endl;
}

std::string *WeatherLine::getName() { return &this->name; }

Magick::Image *WeatherLine::getIcon() {
  return &(*this->image_map)[this->weather_image];
}

void WeatherLine::render(FrameCanvas *offscreen_canvas, char opacity) {
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
void WeatherLine::update() {
  if (!is_visible) {
    return;
  }

  const auto now = std::chrono::system_clock::now();
  bool screen_rotated = false;

  if (((now - this->last_weather_update) / 1s) >
      this->update_weather_after_seconds) {
    std::cout << "Fetching wether data from " << this->url << std::endl;
    try {
      Json::Value jsonData = fetcher->fetch(this->url);

      const std::string condition(
          jsonData["weather"][0]["description"].asString());
      const std::string weather_icon(jsonData["weather"][0]["icon"].asString());
      const double kevinScale = 273.15;
      const double temp = jsonData["main"]["temp"].asDouble() - kevinScale;

      std::stringstream temp_str_stream;
      temp_str_stream << std::fixed << std::setprecision(0) << temp;
      std::string temp_str = temp_str_stream.str();

      std::cout << "\tCondition: " << condition << std::endl;
      std::cout << "\tTemp: " << temp << std::endl;
      std::cout << "\tIcon: " << weather_icon << std::endl;

      std::cout << std::endl;

      this->weather_image.clear();
      this->weather_image.append(weather_icon);
      this->current_line.clear();
      this->current_line.append(temp_str).append("℃");
      this->last_weather_update = now;
    } catch (std::runtime_error &e) {
      printf("Failed to fetch Weather\n");
    }
  }
}
