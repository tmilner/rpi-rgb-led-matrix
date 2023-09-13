#ifndef UPDATEABLE_SCREEN_HPP
#define UPDATEABLE_SCREEN_HPP

#include "screen.h"
#include "led-matrix.h"

class UpdateableScreen : public Screen
{
public:
    UpdateableScreen(ScreenSettings settings);
    void update();
    void render(rgb_matrix::Canvas *offscreen_canvas);
};

#endif /*UPDATEABLE_SCREEN_HPP*/