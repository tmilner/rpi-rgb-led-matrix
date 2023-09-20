#include "rotating-box.h"
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

using std::max;
using std::min;

RotatingBox::RotatingBox(rgb_matrix::FrameCanvas *offscreen_canvas) : name{std::string("Rotating Box")},
                                                                      cent_x(offscreen_canvas->width() / 2),
                                                                      cent_y(offscreen_canvas->height() / 2),
                                                                      rotate_square(std::min(offscreen_canvas->width(), offscreen_canvas->height()) * 1.41),
                                                                      min_rotate(cent_x - rotate_square / 2),
                                                                      max_rotate(cent_x + rotate_square / 2),
                                                                      display_square(std::min(offscreen_canvas->width(), offscreen_canvas->height()) * 0.7),
                                                                      min_display(cent_x - display_square / 2),
                                                                      max_display(cent_x + display_square / 2),
                                                                      rotation(0)
{
    return;
}

std::string *RotatingBox::getName()
{
    return &this->name;
}

void RotatingBox::render(rgb_matrix::FrameCanvas *offscreen_canvas)
{
    if (!is_visible)
    {
        return;
    }

    ++this->rotation;
    this->rotation %= 360;
    for (int x = this->min_rotate; x < this->max_rotate; ++x)
    {
        for (int y = this->min_rotate; y < this->max_rotate; ++y)
        {
            float rot_x, rot_y;
            Rotate(x - this->cent_x, y - this->cent_x,
                   this->deg_to_rad * this->rotation, &rot_x, &rot_y);
            if (x >= this->min_display && x < this->max_display &&
                y >= this->min_display && y < this->max_display)
            { // within display square
                offscreen_canvas->SetPixel(rot_x + this->cent_x, rot_y + this->cent_y,
                                           scale_col(x, this->min_display, this->max_display),
                                           255 - scale_col(y, this->min_display, this->max_display),
                                           scale_col(y, this->min_display, this->max_display));
            }
            else
            {
                // black frame.
                offscreen_canvas->SetPixel(rot_x + this->cent_x, rot_y + this->cent_y, 0, 0, 0);
            }
        }
    }
}
uint8_t RotatingBox::scale_col(int val, int lo, int hi)
{
    if (val < lo)
        return 0;
    if (val > hi)
        return 255;
    return 255 * (val - lo) / (hi - lo);
}

void RotatingBox::Rotate(int x, int y, float angle,
                         float *new_x, float *new_y)
{
    *new_x = x * cosf(angle) - y * sinf(angle);
    *new_y = x * sinf(angle) + y * cosf(angle);
}