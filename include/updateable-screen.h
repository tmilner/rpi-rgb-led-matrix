#ifndef UPDATEABLE_SCREEN_HPP
#define UPDATEABLE_SCREEN_HPP

#include "screen.h"
#include "led-matrix.h"

class UpdateableScreen : public Screen
{
public:
    UpdateableScreen(){};
    virtual ~UpdateableScreen(){};
    virtual void update() = 0;
    virtual void render(rgb_matrix::FrameCanvas *offscreen_canvas) = 0;
protected:
    std::string name;
};

#endif /*UPDATEABLE_SCREEN_HPP*/