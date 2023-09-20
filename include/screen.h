#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "led-matrix.h"
#include <string>
#include "renderable.h"

struct ScreenSettings
{
    int width;
    int height;
    rgb_matrix::Font *font;
    rgb_matrix::Color color;
};

class Screen : public Renderable
{
public:
    virtual void render(rgb_matrix::FrameCanvas *offscreen_canvas) = 0;
    virtual std::string *getName() = 0;
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