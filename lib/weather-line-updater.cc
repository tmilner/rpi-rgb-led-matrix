#include "weather-line-updater.h"
#include <iostream>
#include <iomanip>
#include "img_utils.h"

WeatherLineUpdater::WeatherLineUpdater(const std::string weather_api_key, JSONFetcher *fetcher,
                                       std::map<std::string, Magick::Image> *image_map, ScrollingLineSettings settings) : ScrollingLine(settings)
{
    const std::string weather_base_url("https://api.openweathermap.org/data/2.5/weather?lon=-0.093014&lat=51.474087&appid=");

    this->fetcher = fetcher;
    this->line = "Loading";
    this->url = weather_base_url + weather_api_key;
    this->refreshCount = 9; // Refresh after one loop.
    this->image_map = image_map;
    this->current_image = "01d";
}

Magick::Image *WeatherLineUpdater::getIcon()
{
    return &(*this->image_map)[this->current_image];
}

void WeatherLineUpdater::render(FrameCanvas *offscreen_canvas)
{
    std::cout << "RENDER - WEATHER - Y = " << this->y << std::endl;
    this->updateText(this->getLine());
    this->renderLine(offscreen_canvas);
    offscreen_canvas->SetPixels(0, this->y, 13, 16, 0, 0, 0);
    rgb_matrix::DrawLine(offscreen_canvas, 13, this->y, 13, this->y + 16, Color(130, 100, 73));
    CopyImageToCanvas(this->getIcon(), offscreen_canvas, 0, this->y);
}
void WeatherLineUpdater::update()
{
    if (this->refreshCount == 10)
    {
        this->refreshCount = 0;
        std::cout << "Fetching wether data from " << this->url << std::endl;
        try
        {
            Json::Value jsonData = fetcher->fetch(this->url);

            const std::string condition(jsonData["weather"][0]["description"].asString());
            const std::string weather_icon(jsonData["weather"][0]["icon"].asString());
            const double kevinScale = 273.15;
            const double temp = jsonData["main"]["temp"].asDouble() - kevinScale;

            std::stringstream temp_str_stream;
            temp_str_stream << std::fixed << std::setprecision(0) << temp;
            std::string temp_str = temp_str_stream.str();

            std::cout << "\tCondition: " << condition << std::endl;
            std::cout << "\tTemp: " << temp << std::endl;
            std::cout << "\tIcon: " << weather_icon << std::endl;

            std::cout << std::endl;

            this->current_image.clear();
            this->current_image.append(weather_icon);
            this->line.clear();
            this->line.append(temp_str).append("℃");
        }
        catch (std::runtime_error &e)
        {
            printf("Failed to fetch radio6\n");
        }
    }
    this->refreshCount++;
}
