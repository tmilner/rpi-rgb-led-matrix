#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "led-matrix.h"

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
    void set_visible()
    {
        is_visible = true;
    }
    void set_hidden()
    {
        is_visible = false;
    }

protected:
    bool is_visible;
};

#endif /*SCREEN_HPP*/