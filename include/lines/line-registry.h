#ifndef LINE_REGISTRY_H
#define LINE_REGISTRY_H

#include "core/service-registry.h"
#include "lines/scrolling-line.h"
#include "screens/updatable-screen.h"

#include <Magick++.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

enum class LineType {
  Radio6,
  CurrentTime,
  CurrentDate,
  Weather,
  TimeDateWeather,
  Bus
};

struct LineRegistryContext {
  std::shared_ptr<std::map<std::string, Magick::Image>> image_map;
  std::map<std::string, std::string> weather_icon_map;
  std::string weather_api_key;
  ServiceRegistry *services = nullptr;
};

using LineMap = std::map<LineType, std::unique_ptr<UpdatableScreen>>;

LineMap BuildLineMap(const std::vector<LineType> &options,
                     const ScrollingLineSettings &settings,
                     const LineRegistryContext &context);

UpdatableScreen *GetLine(LineMap &lines, LineType type);
void ResetLinePosition(UpdatableScreen *line);

#endif /*LINE_REGISTRY_H*/
