// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to scroll text.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

// For a utility with a few more features see
// ../utils/text-scroller.cc

#include "led-matrix.h"
#include "graphics.h"
#include "rotary_dial.h"
#include "json-fetcher.h"
#include "scrolling-line.h"
#include "img_utils.h"
#include "screen_state.h"
#include "radio6-line-updater.h"
#include "weather-line-updater.h"

#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <filesystem>
#include <stdexcept>
#include <iomanip>
#include <sstream>

#include <cppgpio.hpp>
#include <yaml-cpp/yaml.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <json/json.h>

#include <Magick++.h>
#include <magick/image.h>

namespace fs = std::filesystem;

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
  interrupt_received = true;
}

using namespace std::literals::chrono_literals;
void updateLines(WeatherLineUpdater *weather, Radio6LineUpdater *radio6)
{
  while (true)
  {
    radio6->updateLine();
    weather->updateLine();
    std::this_thread::sleep_for(30s);
  }
}

int main(int argc, char *argv[])
{
  ScreenState state;
  state.image_map = {};

  state.line1str = "Loading";
  state.line2str = "Loading";

  state.menu_items = {"Brightness", "Exit"};
  state.current_mode = ScreenMode::display;
  state.current_menu_item = 0;
  state.current_brightness = 100;

  Magick::InitializeMagick(*argv);
  RotaryDialWithPush dial(state.current_mode, state.current_menu_item, state.menu_items, state.current_brightness);

  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "adafruit-hat-pwm";
  defaults.rows = 32;
  defaults.cols = 64;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = false;

  RGBMatrix *matrix = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
  if (matrix == NULL)
    return 1;

  // Create a new canvas to be used with led_matrix_swap_on_vsync
  FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();
  FrameCanvas *menu_offscreen_canvas = matrix->CreateFrameCanvas();

  Color color(240, 160, 100);
  Color divider_color(130, 100, 73);

  Color menu_bg_color(60, 60, 60);
  Color bg_color(0, 0, 0);

  rgb_matrix::DrawCircle(offscreen_canvas, 32, 14, 10, color);
  offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);

  /* x_origin is set by default just right of the screen */
  const int width = defaults.chain_length * defaults.cols;

  int letter_spacing = 0;
  float speed = 3.0f;
  const char *base_path_c = ".";

  int opt;
  while ((opt = getopt(argc, argv, "s:p:")) != -1)
  {
    switch (opt)
    {
    case 's':
      speed = atof(optarg);
      break;
      break;
    case 'p':
      base_path_c = strdup(optarg);
      break;
    }
  }

  const std::string base_path(base_path_c);

  // Weather API Key
  YAML::Node config = YAML::LoadFile(base_path + "/config.yaml");
  const std::string weather_api_key = config["weather_api_key"].as<std::string>();

  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  const std::string bdf_8x13_font_path(base_path + "/fonts/8x13.bdf");
  const std::string bdf_5x7_font_path(base_path + "/fonts/5x7.bdf");

  rgb_matrix::Font main_font;
  if (!main_font.LoadFont(bdf_8x13_font_path.c_str()))
  {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_8x13_font_path);
    return 1;
  }
  rgb_matrix::Font menu_font;
  if (!menu_font.LoadFont(bdf_5x7_font_path.c_str()))
  {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_5x7_font_path);
    return 1;
  }

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  printf("CTRL-C for exit.\n");

  rgb_matrix::DrawText(offscreen_canvas, main_font,
                       4, 8 + main_font.baseline(),
                       color, nullptr,
                       "Loading", letter_spacing);
  offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);

  int delay_speed_usec = 1000000;
  if (speed > 0)
  {
    delay_speed_usec = 1000000 / speed / main_font.CharacterWidth('W');
  }

  const std::string image_path("/img");

  for (auto const &dir_entry : std::filesystem::directory_iterator{base_path + image_path})
  {
    if (dir_entry.path().extension() == ".png")
    {
      ImageVector image = LoadImageAndScaleImage(
          (dir_entry.path()).c_str(),
          13,
          13);

      std::cout << "Loading image: " << dir_entry.path() << std::endl;

      if (image.size() == 0)
      {
        std::cout << "FAILED TO LOAD IMAGE" << dir_entry.path() << std::endl;
      }
      else
      {
        state.image_map[dir_entry.path().stem()] = image[0];
      }
    }
    else
    {
      std::cout << "Found a non PNG file: " << dir_entry.path() << ", extension is: " << dir_entry.path().extension() << std::endl;
    }
  }

  std::cout << "Images loaded: " << std::endl;
  for (const auto &[key, value] : state.image_map)
    std::cout << '[' << key << "] = " << value.constImageInfo() << std::endl;

  std::string current_image = "01d";

  JSONFetcher *fetcher{};

  ScrollingLineSettings weatherLineSettings = ScrollingLineSettings(
      speed,
      18,
      letter_spacing,
      &main_font,
      color,
      width,
      14);

  Radio6LineUpdater radio6 = Radio6LineUpdater(fetcher, &state.image_map);
  WeatherLineUpdater weather = WeatherLineUpdater(weather_api_key, fetcher, &state.image_map, weatherLineSettings);

  std::thread updateThread(updateLines, &weather, &radio6);

  ScrollingLine line1(ScrollingLineSettings(
      speed,
      1,
      letter_spacing,
      &main_font,
      color,
      width,
      14));

  ScrollingLine menu_line(ScrollingLineSettings(
      speed,
      9,
      letter_spacing,
      &menu_font,
      color,
      width,
      0));

  ScrollingLine menu_sub_line(ScrollingLineSettings(
      speed,
      17,
      letter_spacing,
      &menu_font,
      color,
      width,
      0));

  offscreen_canvas->Clear();

  while (!interrupt_received)
  {
    if (state.current_mode == display)
    {
      offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);

      line1.updateText(radio6.getLine());
      weather.render(offscreen_canvas);
      // line2.updateText(weather.getLine());
      line1.renderLine(offscreen_canvas);
      // line2.render(offscreen_canvas);

      offscreen_canvas->SetPixels(0, 0, 13, 32, 0, 0, 0);

      rgb_matrix::DrawLine(offscreen_canvas, 13, 0, 13, 32, divider_color);

      CopyImageToCanvas(radio6.getIcon(), offscreen_canvas, 1, 2);
      // CopyImageToCanvas(weather.getIcon(), offscreen_canvas, 0, 18);

      // Swap the offscreen_canvas with canvas on vsync, avoids flickering
      offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
      usleep(delay_speed_usec);
    }
    else
    {
      offscreen_canvas->SetBrightness(state.current_brightness);
      menu_offscreen_canvas->SetBrightness(state.current_brightness);

      menu_offscreen_canvas->CopyFrom(*offscreen_canvas);

      if (state.current_mode == main_menu)
      {
        menu_offscreen_canvas->SetPixels(0, 7, width, defaults.rows - 13, 50, 50, 50);
      }
      else
      {
        menu_offscreen_canvas->SetPixels(0, 7, width, defaults.rows - 13, 233, 110, 80);
        menu_offscreen_canvas->SetPixels(1, 8, width - 2, defaults.rows - 15, 50, 50, 50);
        menu_sub_line.updateText(&std::to_string(state.current_brightness).append("%"));
        menu_sub_line.renderLine(menu_offscreen_canvas);
      }
      menu_line.updateText(&state.menu_items[state.current_menu_item]);
      menu_line.renderLine(menu_offscreen_canvas);

      matrix->SetBrightness(state.current_brightness);

      menu_offscreen_canvas = matrix->SwapOnVSync(menu_offscreen_canvas);
      usleep(delay_speed_usec);
    }
  }

  // Finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;
  delete fetcher;

  return 0;
}
