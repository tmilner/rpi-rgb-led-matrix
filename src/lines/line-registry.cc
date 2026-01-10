#include "lines/line-registry.h"

#include "lines/bus-arrivals-line.h"
#include "lines/date-line.h"
#include "lines/music-info-line.h"
#include "lines/time-date-weather-line.h"
#include "lines/time-line.h"
#include "lines/weather-line.h"

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
