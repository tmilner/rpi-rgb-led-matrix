#include "bus-towards-oval-line.h"
#include <iostream>
#include "img_utils.h"

using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

BusTowardsOvalLine::BusTowardsOvalLine(std::map<std::string, Magick::Image> *image_map, TflClient tflClient, ScrollingLineSettings settings) : ScrollingLine(settings),
                                                                                                                                               tflClient(tflClient),
                                                                                                                                               name{std::string("Buses Towards Oval")}
{
    this->current_line = "Loading";
    this->image_map = image_map;
    this->image_key = "bus";
    this->is_visible = true;
    auto now = std::chrono::system_clock::now();
    this->last_update = now - 5min;
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
    CopyImageToCanvas(this->getIcon(), offscreen_canvas, 0, this->y + 1);
    rgb_matrix::DrawLine(offscreen_canvas, 13, this->y, 13, this->y + 16, Color(130, 100, 73));
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

    try
    {
        std::vector<TflClient::Arrival> arrivals = this->tflClient.getButArrivals("490014229J");
        std::string busTimes = "";

        for (auto &arrival : arrivals)
        {
            busTimes.append(arrival.getDisplayString()).append("-");
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
