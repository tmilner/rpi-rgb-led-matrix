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
    WeatherLineUpdater(const std::string weather_api_key, JSONFetcher *fetcher, std::map<std::string, Magick::Image> *image_map, ScrollingLineSettings settings);
    void update();
    void render(FrameCanvas *offscreen_canvas);

private:
    Magick::Image *getIcon();
    JSONFetcher *fetcher;
    std::string url;
    std::string current_image;
    int refreshCount;
    std::map<std::string, Magick::Image> *image_map{};
};

#endif /*WEATHER_LINE_UPDATER_H*/