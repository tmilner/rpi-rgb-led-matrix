#ifndef SCROLLING_SCREEN_H
#define SCROLLING_SCREEN_H
#include "updateable-screen.h"
#include "scrolling-line.h"
#include "json-fetcher.h"
#include <Magick++.h>
#include "spotify_client.h"

enum ScreenLineOption
{
    radio6,
    weather
};

struct ScrollingLineScreenSettings : ScreenSettings
{
    int width;
    int height;
    rgb_matrix::Font *font;
    rgb_matrix::Color color;
    rgb_matrix::Color bg_color;
    float *speed;
    int letter_spacing;
    ScreenLineOption line1;
    ScreenLineOption line2;
    std::string weather_api_key;
    ScrollingLineScreenSettings(int width,
                                int height,
                                rgb_matrix::Font *font,
                                rgb_matrix::Color color,
                                rgb_matrix::Color bg_color,
                                float *speed,
                                int letter_spacing,
                                ScreenLineOption line1,
                                ScreenLineOption line2,
                                const std::string weather_api_key)
    {
        this->width = width;
        this->height = height;
        this->font = font;
        this->color = color;
        this->bg_color = bg_color;
        this->speed = speed;
        this->line1 = line1;
        this->line2 = line2;
        this->weather_api_key = weather_api_key;
    }
};

class ScrollingLineScreen : public UpdateableScreen
{
public:
    ScrollingLineScreen(std::map<std::string, Magick::Image> *image_map, ScrollingLineScreenSettings settings,
                        SpotifyClient spotify_client);
    void update();
    void render(FrameCanvas *offscreen_canvas);
    void setLine1(ScreenLineOption type);
    void setLine2(ScreenLineOption type);
    std::string *getName();

private:
    ScrollingLineScreenSettings settings;
    SpotifyClient spotify_client;
    rgb_matrix::Color bg_color;
    std::string name;
    std::map<std::string, Magick::Image> *image_map{};
    UpdateableScreen *line1;
    UpdateableScreen *line2;
    ScrollingLineSettings line1_settings;
    ScrollingLineSettings line2_settings;
};
#endif /*SCROLLING_SCREEN_H*/