#include "line-updater.h"
#include "led-matrix.h"
#include "scrolling-line.h"

class WeatherLineUpdater : public LineUpdater, ScrollingLine
{
public:
    WeatherLineUpdater(const std::string weather_api_key, JSONFetcher *fetcher, std::map<std::string, Magick::Image> *image_map, ScrollingLineSettings settings);
    void updateLine();
    Magick::Image *getIcon();
    void render(FrameCanvas *offscreen_canvas);

private:
    JSONFetcher *fetcher;
    std::string url;
    std::string current_image;
    int y_pos;
    int refreshCount;
    std::map<std::string, Magick::Image> *image_map{};
};
