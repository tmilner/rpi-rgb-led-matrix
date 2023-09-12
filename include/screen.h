#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "led-matrix.h"

class Screen
{
public:
    void render(rgb_matrix::Canvas *offscreen_canvas);
    void set_visible() {
        is_visible = true;
    }
    void set_hidden() {
        is_visible = false;
    }
protected:
    bool is_visible;
};

#endif /*SCREEN_HPP*/