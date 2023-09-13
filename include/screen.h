#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "led-matrix.h"

struct ScreenSettings {
    int width;
    int height;
    rgb_matrix::Font *font;
    rgb_matrix::Color color;
};

class Screen
{
public:
    Screen(ScreenSettings settings);
    void render(rgb_matrix::Canvas *offscreen_canvas);
    void set_visible() {
        is_visible = true;
    }
    void set_hidden() {
        is_visible = false;
    }
protected:
    bool is_visible;
    ScreenSettings settings;
};

#endif /*SCREEN_HPP*/