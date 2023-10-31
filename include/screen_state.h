#ifndef SCREEN_STATE_H
#define SCREEN_STATE_H

#include <string>
#include <map>
#include <vector>

#include <Magick++.h>

#include "screen_mode.h"

namespace rgb_matrix
{
    struct ScreenState
    {
        std::map<std::string, Magick::Image> image_map{};
        ScreenMode current_mode = ScreenMode::scrolling_lines;
        int current_brightness = 100;
        float speed = 2.0f;
    };
}
#endif /*SCREEN_STATE_H*/