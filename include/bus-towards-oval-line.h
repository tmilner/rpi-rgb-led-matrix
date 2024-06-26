#ifndef BUS_TOWARDS_OVAL_LINE_H
#define BUS_TOWARDS_OVAL_LINE_H
#include "updateable-screen.h"
#include "scrolling-line.h"
#include "json-fetcher.h"
#include "tfl-client.h"
#include <Magick++.h>
#include <chrono>

class BusTowardsOvalLine : public UpdateableScreen, ScrollingLine
{
public:
    BusTowardsOvalLine(std::map<std::string, Magick::Image> *image_map, TflClient tflClient, ScrollingLineSettings settings);
    void update();
    void render(FrameCanvas *offscreen_canvas);
    std::string *getName();

private:
    Magick::Image *getIcon();
    TflClient tflClient;
    std::chrono::time_point<std::chrono::system_clock> last_update;
    static const int update_after_seconds = 20;
    std::map<std::string, Magick::Image> *image_map{};
    std::string image_key;
    std::string name;
};
#endif /*BUS_TOWARDS_OVAL_LINE_H*/