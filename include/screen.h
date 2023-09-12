#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "led-matrix.h"

class Screen
{
public:
    void render(rgb_matrix::Canvas *offscreen_canvas);
};

#endif /*SCREEN_HPP*/