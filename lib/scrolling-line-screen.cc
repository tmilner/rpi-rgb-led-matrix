#include "scrolling-line-screen.h"
#include "radio6-line-updater.h"
#include "weather-line-updater.h"
#include <iostream>
#include "img_utils.h"

ScrollingLineScreen::ScrollingLineScreen(std::map<std::string, Magick::Image> *image_map, ScrollingLineScreenSettings settings) : image_map{image_map}, line1_settings{settings.speed,
                                                                                                                                                                       0,
                                                                                                                                                                       0,
                                                                                                                                                                       settings.font,
                                                                                                                                                                       settings.color,
                                                                                                                                                                       settings.width,
                                                                                                                                                                       14},
                                                                                                                                  line2_settings{settings.speed,
                                                                                                                                                 settings.height / 2,
                                                                                                                                                 0,
                                                                                                                                                 settings.font,
                                                                                                                                                 settings.color,
                                                                                                                                                 settings.width, 14},
                                                                                                                                  settings{settings},
                                                                                                                                  bg_color{settings.bg_color},
                                                                                                                                  name{std::string("Scrolling Screen")}

{
    this->image_map = image_map;
    this->is_visible = true;

    Radio6LineUpdater *radio6LineUpdater = new Radio6LineUpdater(image_map, line1_settings);
    this->line1 = radio6LineUpdater;
    WeatherLineUpdater *weatherLineUpdater = new WeatherLineUpdater(settings.weather_api_key, image_map, line2_settings);
    this->line2 = weatherLineUpdater;
}

std::string *ScrollingLineScreen::getName()
{
    return &this->name;
}

void ScrollingLineScreen::render(FrameCanvas *offscreen_canvas)
{
    if (!is_visible)
    {
        return;
    }
    offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
    this->line1->render(offscreen_canvas);
    this->line2->render(offscreen_canvas);
}

void ScrollingLineScreen::setLine1(ScreenLineOption type)
{

    if (type == ScreenLineOption::radio6)
    {
        Radio6LineUpdater *radio6LineUpdater = new Radio6LineUpdater(this->image_map, this->line1_settings);
        delete this->line1;
        this->line1 = radio6LineUpdater;
    }
    else
    {
        WeatherLineUpdater *weatherLineUpdater = new WeatherLineUpdater(this->settings.weather_api_key, this->image_map, this->line1_settings);
        delete this->line1;
        this->line1 = weatherLineUpdater;
    }
}
void ScrollingLineScreen::setLine2(ScreenLineOption type)
{
    if (type == ScreenLineOption::radio6)
    {
        Radio6LineUpdater *radio6LineUpdater = new Radio6LineUpdater(this->image_map, this->line2_settings);
        delete this->line2;
        this->line2 = radio6LineUpdater;
    }
    else
    {
        WeatherLineUpdater *weatherLineUpdater = new WeatherLineUpdater(this->settings.weather_api_key, this->image_map, this->line2_settings);
        delete this->line2;
        this->line2 = weatherLineUpdater;
    }
}
void ScrollingLineScreen::update()
{
    if (!is_visible)
    {
        return;
    }
    this->line1->update();
    this->line2->update();
}
