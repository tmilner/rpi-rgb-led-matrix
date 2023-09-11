#include "line-updater.h"

class Radio6LineUpdater : public LineUpdater
{
public:
    Radio6LineUpdater(JSONFetcher *fetcher,
                      std::map<std::string, Magick::Image> *image_map);
    void updateLine();
    Magick::Image *getIcon();

private:
    JSONFetcher *fetcher;
    std::string url;
    std::map<std::string, Magick::Image> *image_map{};
    std::string image_key;
};
