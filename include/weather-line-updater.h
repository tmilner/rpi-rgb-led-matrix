#ifndef WEATHER_LINE_UPDATER_H
#define WEATHER_LINE_UPDATER_H
#include "updateable-screen.h"
#include "led-matrix.h"
#include "scrolling-line.h"
#include "json-fetcher.h"
#include <Magick++.h>

class WeatherLineUpdater : public UpdateableScreen, ScrollingLine
{
public:
    WeatherLineUpdater(const std::string weather_api_key, std::map<std::string, Magick::Image> *image_map, ScrollingLineSettings settings);
    void update();
    void render(FrameCanvas *offscreen_canvas);
    std::string *getName();

private:
    Magick::Image *getIcon();
    JSONFetcher *fetcher;
    std::string url;
    std::string current_image;
    int refreshCount;
    std::map<std::string, Magick::Image> *image_map{};
    std::string name;
};

#endif /*WEATHER_LINE_UPDATER_H*/