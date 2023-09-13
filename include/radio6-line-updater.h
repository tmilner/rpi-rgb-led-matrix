#ifndef RADIO6_LINE_UDPATER_H
#define RADIO6_LINE_UPDATER_H
#include "updateable-screen.h"
#include "scrolling-line.h"
#include "json-fetcher.h"
#include <Magick++.h>

class Radio6LineUpdater : public UpdateableScreen, ScrollingLine
{
public:
    Radio6LineUpdater(JSONFetcher *fetcher,
                      std::map<std::string, Magick::Image> *image_map, ScrollingLineSettings settings);
    void update();
    void render(FrameCanvas *offscreen_canvas);

private:
    Magick::Image *getIcon();
    JSONFetcher *fetcher;
    std::string url;
    std::map<std::string, Magick::Image> *image_map{};
    std::string image_key;
};
#endif /*RADIO6_LINE_UPDATER_H*/