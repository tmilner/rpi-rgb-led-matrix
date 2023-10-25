// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to scroll text.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

// For a utility with a few more features see
// ../utils/text-scroller.cc

#include "led-matrix.h"
#include "graphics.h"
#include "json-fetcher.h"
#include "scrolling-line.h"
#include "img_utils.h"
#include "screen_state.h"
#include "radio6-line-updater.h"
#include "weather-line-updater.h"
#include "scrolling-line-screen.h"
#include "rotating-box.h"
#include "screen-menu.h"
#include "game-of-life.h"
#include "updateable-screen.h"

#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <filesystem>
#include <stdexcept>
#include <iomanip>
#include <sstream>

#include <list>
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
void updateLines(std::vector<UpdateableScreen *> screens_to_update)
{
  while (true)
  {
    for (std::vector<UpdateableScreen *>::iterator screen = screens_to_update.begin(); screen != screens_to_update.end(); screen++)
    {
      (*screen)->update();
    }
    usleep(20 * 1000 * 1000);
  }
}

int main(int argc, char *argv[])
{
  ScreenState state;
  state.image_map = {};
  state.current_mode = ScreenMode::scrolling_lines;
  state.current_brightness = 100;

  // GPIO::RotaryDial dial(25, 9, GPIO::GPIO_PULL::UP);
  GPIO::PushButton push_ok(11, GPIO::GPIO_PULL::UP);
  GPIO::PushButton push_up(25, GPIO::GPIO_PULL::UP);
  GPIO::PushButton push_down(9, GPIO::GPIO_PULL::UP);

  Magick::InitializeMagick(*argv);

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

  Color color(240, 160, 100);
  Color divider_color(130, 100, 73);

  Color menu_bg_color(60, 60, 60);
  Color bg_color(0, 0, 0);

  rgb_matrix::DrawCircle(offscreen_canvas, 32, 13, 10, color);
  offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);

  /* x_origin is set by default just right of the screen */
  const int width = defaults.chain_length * defaults.cols;

  int letter_spacing = 0;
  float speed = 1.0f;
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

  ScrollingLineScreenSettings scrollingLineScreenSettings = ScrollingLineScreenSettings(defaults.cols,
                                                                                        defaults.rows,
                                                                                        &main_font,
                                                                                        color,
                                                                                        bg_color,
                                                                                        speed,
                                                                                        letter_spacing,
                                                                                        ScreenLineOption::radio6,
                                                                                        ScreenLineOption::weather,
                                                                                        weather_api_key);

  ScrollingLineScreen *srollingTwoLineScreen = new ScrollingLineScreen(&state.image_map, scrollingLineScreenSettings);

  std::cout << "Setting up update thread" << std::endl;

  GameOfLfeScreen *game_of_life_screen = new GameOfLfeScreen(offscreen_canvas, 500, true);
  game_of_life_screen->set_hidden();

  RotatingBox *rotating_box = new RotatingBox(offscreen_canvas);
  rotating_box->set_hidden();

  std::vector<Screen *> screens_to_render;

  screens_to_render.push_back(srollingTwoLineScreen);
  screens_to_render.push_back(game_of_life_screen);
  screens_to_render.push_back(rotating_box);

  std::vector<UpdateableScreen *> screens_to_update;

  screens_to_update.push_back(srollingTwoLineScreen);
  screens_to_update.push_back(game_of_life_screen);

  std::thread updateThread(updateLines, screens_to_update);

  ScreenMenu menu = ScreenMenu(
      speed,
      letter_spacing,
      &menu_font,
      width,
      &state,
      &push_ok,
      &push_up,
      &push_down,
      &screens_to_render);

  offscreen_canvas->Clear();

  while (!interrupt_received)
  {
    offscreen_canvas->SetBrightness(state.current_brightness);
    offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);

    for (std::vector<Screen *>::iterator screen = screens_to_render.begin(); screen != screens_to_render.end(); screen++)
    {
      (*screen)->render(offscreen_canvas);
    }

    menu.render(offscreen_canvas);
    offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
    usleep(delay_speed_usec);
  }

  // Finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  return 0;
}
