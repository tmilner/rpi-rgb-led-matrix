#ifndef WEATHER_LINE_UPDATER_H
#define WEATHER_LINE_UPDATER_H
#include "updateable-screen.h"
#include "led-matrix.h"
#include "scrolling-line.h"
#include "json-fetcher.h"
#include <Magick++.h>
#include <chrono>

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
    std::string weather_line;
    std::string weather_image;
    std::string time_image;
    std::string date_image;
    std::chrono::time_point<std::chrono::system_clock> last_weather_update;
    std::chrono::time_point<std::chrono::system_clock> last_rotate;
    int current_display;
    static const int rotate_after_seconds = 15;
    static const int update_weather_after_seconds = 360;
    std::map<std::string, Magick::Image> *image_map{};
    std::string name;
};

#endif /*WEATHER_LINE_UPDATER_H*/