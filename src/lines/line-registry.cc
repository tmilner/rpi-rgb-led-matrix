#include "lines/line-registry.h"

#include "lines/bus-arrivals-line.h"
#include "lines/date-line.h"
#include "lines/music-info-line.h"
#include "lines/time-date-weather-line.h"
#include "lines/time-line.h"
#include "lines/weather-line.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace {
std::string toLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return value;
}

std::string trim(const std::string &value) {
  size_t start = value.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = value.find_last_not_of(" \t\n\r");
  return value.substr(start, end - start + 1);
}
} // namespace

LineMap BuildLineMap(const std::vector<LineType> &options,
                     const ScrollingLineSettings &settings,
                     const LineRegistryContext &context) {
  LineMap lines;
  if (context.services == nullptr) {
    return lines;
  }
  for (LineType option : options) {
    if (lines.find(option) != lines.end()) {
      continue;
    }
    switch (option) {
    case LineType::Radio6:
      lines.emplace(option,
                    std::make_unique<MusicInfoLine>(
                        context.image_map, &context.services->spotify(),
                        &context.services->radio6(), settings));
      break;
    case LineType::Bus:
      lines.emplace(option,
                    std::make_unique<BusArrivalsLine>(
                        context.image_map, &context.services->tfl(), settings));
      break;
    case LineType::CurrentTime:
      lines.emplace(option,
                    std::make_unique<TimeLine>(context.image_map, settings));
      break;
    case LineType::CurrentDate:
      lines.emplace(option,
                    std::make_unique<DateLine>(context.image_map, settings));
      break;
    case LineType::Weather:
      lines.emplace(option,
                    std::make_unique<WeatherLine>(
                        context.weather_api_key, context.weather_icon_map,
                        context.image_map, settings));
      break;
    case LineType::TimeDateWeather:
      lines.emplace(option,
                    std::make_unique<TimeDateWeatherLine>(
                        context.weather_api_key, context.image_map.get(),
                        settings));
      break;
    }
  }
  return lines;
}

bool TryParseLineType(const std::string &value, LineType &type) {
  const std::string key = toLower(trim(value));
  if (key == "radio6") {
    type = LineType::Radio6;
    return true;
  }
  if (key == "time") {
    type = LineType::CurrentTime;
    return true;
  }
  if (key == "date") {
    type = LineType::CurrentDate;
    return true;
  }
  if (key == "weather") {
    type = LineType::Weather;
    return true;
  }
  if (key == "timedateweather") {
    type = LineType::TimeDateWeather;
    return true;
  }
  if (key == "bus") {
    type = LineType::Bus;
    return true;
  }
  return false;
}

std::string LineTypeToString(LineType type) {
  switch (type) {
  case LineType::Radio6:
    return "radio6";
  case LineType::CurrentTime:
    return "time";
  case LineType::CurrentDate:
    return "date";
  case LineType::Weather:
    return "weather";
  case LineType::TimeDateWeather:
    return "timedateweather";
  case LineType::Bus:
    return "bus";
  }
  return "unknown";
}

UpdatableScreen *GetLine(LineMap &lines, LineType type) {
  auto it = lines.find(type);
  if (it == lines.end()) {
    return nullptr;
  }
  return it->second.get();
}

void ResetLinePosition(UpdatableScreen *line) {
  auto scrolling_line = dynamic_cast<ScrollingLine *>(line);
  if (scrolling_line != nullptr) {
    scrolling_line->resetXPosition();
  }
}
