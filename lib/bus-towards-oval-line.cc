#include "bus-towards-oval-line.h"
#include <iostream>
#include "img_utils.h"

using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

BusTowardsOvalLine::BusTowardsOvalLine(std::map<std::string, Magick::Image> *image_map, ScrollingLineSettings settings) : ScrollingLine(settings),
                                                                                                                          name{std::string("Buses Towards Oval")}
{
    this->current_line = "Loading";
    this->url = std::string("https://api.tfl.gov.uk/StopPoint/490014229J/Arrivals");
    this->image_map = image_map;
    this->image_key = "bus";
    this->is_visible = true;
    this->fetcher = new JSONFetcher();
    auto now = std::chrono::system_clock::now();
    this->last_update = now - 5min;

    Magick::Image tmp = (*image_map)[this->image_key];
    tmp.resize(Magick::Geometry(11, 11));

    (*this->image_map)[this->image_key] = tmp;
}

Magick::Image *BusTowardsOvalLine::getIcon()
{
    return &(*this->image_map)[this->image_key];
}

std::string *BusTowardsOvalLine::getName()
{
    return &this->name;
}

void BusTowardsOvalLine::render(FrameCanvas *offscreen_canvas)
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

void BusTowardsOvalLine::update()
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

    std::cout << "Fetching bus towards oval data from " << this->url << std::endl;

    try
    {
        Json::Value jsonData = fetcher->fetch(this->url);
        std::string busTimes = "";

        for (int i = 0; i < jsonData.size(); i++)
        {
            Json::Value bus = jsonData[i];
            int timeToStationSeconds = bus["timeToStation"].asInt();

            if (timeToStationSeconds > 60)
            {
                busTimes.append(bus["lineName"].asString())
                    .append(" ")
                    .append(std::to_string(timeToStationSeconds / 60))
                    .append("mins")
                    .append(" - ");
            }
        }

        std::cout << "\t Next busses: " << busTimes << std::endl;
        std::cout << std::endl;

        this->current_line.clear();
        this->current_line.append(busTimes);
    }
    catch (std::runtime_error &e)
    {
        printf("Failed to fetch Radio6 Data\n");
    }
}
