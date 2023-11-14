#include "radio6-line-updater.h"
#include <iostream>
#include "img_utils.h"

using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

Radio6LineUpdater::Radio6LineUpdater(std::map<std::string, Magick::Image> *image_map, ScrollingLineSettings settings) : ScrollingLine(settings),
                                                                                                                        name{std::string("Radio 6 Line")}
{
    this->current_line = "Loading";
    this->url = std::string("https://nowplaying.jameswragg.com/api/bbc6music?limit=1");
    this->image_map = image_map;
    this->image_key = "radio6icon";
    this->is_visible = true;
    this->fetcher = new JSONFetcher();
    auto now = std::chrono::system_clock::now();
    this->last_update = now - 5min;

    Magick::Image tmp = (*image_map)[this->image_key];
    tmp.resize(Magick::Geometry(11, 11));

    (*this->image_map)[this->image_key] = tmp;
}

Magick::Image *Radio6LineUpdater::getIcon()
{
    return &(*this->image_map)[this->image_key];
}

std::string *Radio6LineUpdater::getName()
{
    return &this->name;
}

void Radio6LineUpdater::render(FrameCanvas *offscreen_canvas)
{
    if (!is_visible)
    {
        return;
    }
    this->renderLine(offscreen_canvas);
    offscreen_canvas->SetPixels(0, this->y, 13, 16, 0, 0, 0);
    rgb_matrix::DrawLine(offscreen_canvas, 13, this->y, 13, this->y + 16, Color(130, 100, 73));
    CopyImageToCanvas(this->getIcon(), offscreen_canvas, 1, this->y + 2);
}

void Radio6LineUpdater::update()
{
    if (!is_visible)
    {
        return;
    }

    const auto now = std::chrono::system_clock::now();

    if (((now - this->last_update) / 1s) < this->update_after_seconds)
    {
        return;
    }

    std::cout << "Fetching radio6 data from " << this->url << std::endl;

    try
    {
        Json::Value jsonData = fetcher->fetch(this->url);

        std::string artist(jsonData["tracks"][0]["artist"].asString());
        std::string track_name(jsonData["tracks"][0]["name"].asString());

        std::cout << "\tArtist: " << artist << std::endl;
        std::cout << "\tTrack Name: " << track_name << std::endl;
        std::cout << std::endl;

        this->last_update = now;

        this->current_line.clear();
        this->current_line.append(artist).append(" - ").append(track_name);
    }
    catch (std::runtime_error &e)
    {
        printf("Failed to fetch Radio6 Data\n");
    }
}
