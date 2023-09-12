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
        std::vector<std::string> menu_items{"Brightness", "Exit"};
        ScreenMode current_mode = ScreenMode::display;
        int current_menu_item = 0;
        int current_brightness = 100;
    };
}