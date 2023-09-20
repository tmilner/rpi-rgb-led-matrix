#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include "led-matrix.h"
#include <string>

class Renderable
{
public:
    virtual void render(rgb_matrix::FrameCanvas *offscreen_canvas) = 0;
};

#endif /*RENDERABLE_HPP*/