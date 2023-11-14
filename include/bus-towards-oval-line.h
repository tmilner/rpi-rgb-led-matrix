#ifndef BUS_TOWARDS_OVAL_LINE_H
#define BUS_TOWARDS_OVAL_LINE_H
#include "updateable-screen.h"
#include "scrolling-line.h"
#include "json-fetcher.h"
#include <Magick++.h>
#include <chrono>

class BusTowardsOvalLine : public UpdateableScreen, ScrollingLine
{
public:
    BusTowardsOvalLine(std::map<std::string, Magick::Image> *image_map, ScrollingLineSettings settings);
    void update();
    void render(FrameCanvas *offscreen_canvas);
    std::string *getName();

private:
    Magick::Image *getIcon();
    JSONFetcher *fetcher;
    std::string url;
    std::chrono::time_point<std::chrono::system_clock> last_update;
    static const int update_after_seconds = 20;
    std::map<std::string, Magick::Image> *image_map{};
    std::string image_key;
    std::string name;
};
#endif /*BUS_TOWARDS_OVAL_LINE_H*/