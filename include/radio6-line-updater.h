#include "line-updater.h"
#include "scrolling-line.h"

class Radio6LineUpdater : public LineUpdater, ScrollingLine
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
