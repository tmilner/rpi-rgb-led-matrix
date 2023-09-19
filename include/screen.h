#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "led-matrix.h"
#include <string>

struct ScreenSettings
{
    int width;
    int height;
    rgb_matrix::Font *font;
    rgb_matrix::Color color;
};

class Screen
{
public:
    virtual void render(rgb_matrix::FrameCanvas *offscreen_canvas) = 0;
    std::string* getName() {
        return &this->name;
    }
    void set_visible()
    {
        is_visible = true;
    }
    void set_hidden()
    {
        is_visible = false;
    }

protected:
    std::string name;
    bool is_visible;
};

#endif /*SCREEN_HPP*/