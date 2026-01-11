// -*- mode: c++; c-basic-offset: 2; indent-t
// Small example how to scroll text.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

// For a utility with a few more features see
// ../utils/text-scroller.cc

#include "clients/mqtt-client.h"
#include "clients/mqtt-controller.h"
#include "core/graphics.h"
#include "core/img_utils.h"
#include "core/led-matrix.h"
#include "core/service-registry.h"
#include "screens/game-of-life-screen.h"
#include "screens/rotating-box-screen.h"
#include "screens/menu-screen.h"
#include "screens/screen_state.h"
#include "screens/scrolling-line-screen.h"
#include "screens/updatable-screen.h"

#include <iostream>
#include <string>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>

#include <cppgpio.hpp>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
// #include <json/json.h>

#include <Magick++.h>
#include <magick/image.h>

using namespace rgb_matrix;
using namespace std;

volatile sig_atomic_t interrupt_received = 0;
static void InterruptHandler(int signo) { interrupt_received = 1; }

using namespace literals::chrono_literals;
void updateLines(vector<UpdatableScreen *> screens_to_update,
                 std::atomic<bool> *running) {
  while (running->load()) {
    for (vector<UpdatableScreen *>::iterator screen =
             screens_to_update.begin();
         screen != screens_to_update.end(); screen++) {
      (*screen)->update();
    }
    usleep(2 * 1000 * 1000);
  }
}

namespace {
template <typename T>
T getOptionalConfig(const YAML::Node &config, const std::string &key,
                    const T &fallback) {
  const YAML::Node value = config[key];
  if (value && !value.IsNull()) {
    return value.as<T>();
  }
  return fallback;
}

std::vector<LineType> parseLineOptions(const YAML::Node &node,
                                       const std::vector<LineType> &fallback) {
  if (!node || node.IsNull()) {
    return fallback;
  }
  std::vector<LineType> options;
  if (node.IsSequence()) {
    for (const auto &entry : node) {
      if (!entry.IsScalar()) {
        continue;
      }
      LineType type = LineType::CurrentTime;
      if (TryParseLineType(entry.as<std::string>(), type)) {
        options.push_back(type);
      }
    }
  } else if (node.IsScalar()) {
    std::stringstream ss(node.as<std::string>());
    std::string item;
    while (std::getline(ss, item, ',')) {
      LineType type = LineType::CurrentTime;
      if (TryParseLineType(item, type)) {
        options.push_back(type);
      }
    }
  }
  if (options.empty()) {
    return fallback;
  }
  return options;
}
} // namespace

int main(int argc, char *argv[]) {
  ScreenState state;
  state.image_map = {};
  state.current_mode = ScreenMode::scrolling_lines;
  state.current_brightness = 50;
  state.speed = 1.5f;
  std::atomic<bool> running(true);

  // GPIO::RotaryDial dial(25, 9, GPIO::GPIO_PULL::UP);
  // GPIO::PushButton push_ok(11, GPIO::GPIO_PULL::UP);
  // GPIO::PushButton push_up(25, GPIO::GPIO_PULL::UP);
  // GPIO::PushButton push_down(9, GPIO::GPIO_PULL::UP);

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
  Color bg_color(0, 0, 0);

  rgb_matrix::DrawCircle(offscreen_canvas, 32, 12, 10, color);
  offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);

  /* x_origin is set by default just right of the screen */
  const int width = defaults.chain_length * defaults.cols;

  int letter_spacing = 0;
  std::string base_path = ".";

  int opt;
  while ((opt = getopt(argc, argv, "p:")) != -1) {
    switch (opt) {
    case 'p':
      base_path = optarg;
      break;
    }
  }

  YAML::Node config = YAML::LoadFile(base_path + "/config.yaml");
  const string weather_api_key =
      getOptionalConfig<std::string>(config, "weather_api_key", "");

  const string spotify_client_id =
      getOptionalConfig<std::string>(config, "spotify_client_id", "");
  const string spotify_client_secret =
      getOptionalConfig<std::string>(config, "spotify_client_secret", "");
  const string spotify_refresh_token =
      getOptionalConfig<std::string>(config, "spotify_refresh_token", "");

  const string mqtt_server =
      getOptionalConfig<std::string>(config, "mqtt_server", "");
  const string mqtt_user_name =
      getOptionalConfig<std::string>(config, "mqtt_user_name", "");
  const string mqtt_password =
      getOptionalConfig<std::string>(config, "mqtt_password", "");
  const string mqtt_client_id =
      getOptionalConfig<std::string>(config, "mqtt_client_id", "led-matrix");

  const string light_state_topic =
      getOptionalConfig<std::string>(config, "light_state_topic", "");
  const string light_command_topic =
      getOptionalConfig<std::string>(config, "light_command_topic", "");
  const string light_brightness_state_topic = getOptionalConfig<std::string>(
      config, "light_brightness_state_topic", "");
  const string light_brightness_command_topic = getOptionalConfig<std::string>(
      config, "light_brightness_command_topic", "");

  const string mqtt_base_topic =
      getOptionalConfig<std::string>(config, "mqtt_base_topic", "led-matrix");
  const string mqtt_discovery_prefix = getOptionalConfig<std::string>(
      config, "mqtt_discovery_prefix", "homeassistant");
  const string mqtt_device_id = getOptionalConfig<std::string>(
      config, "mqtt_device_id", mqtt_client_id);
  const string mqtt_device_name = getOptionalConfig<std::string>(
      config, "mqtt_device_name", "LED Matrix");
  const bool mqtt_enable_discovery =
      getOptionalConfig<bool>(config, "mqtt_enable_discovery", true);

  state.current_brightness = std::clamp(
      getOptionalConfig<int>(config, "brightness", state.current_brightness),
      0, 100);
  state.speed =
      getOptionalConfig<float>(config, "scroll_speed", state.speed);

  YAML::Node weather_icon_map_config =
      YAML::LoadFile(base_path + "/weather_icons.yaml");

  map<string, string> weather_icon_map;

  for (YAML::const_iterator it = weather_icon_map_config.begin();
       it != weather_icon_map_config.end(); ++it) {
    std::string key, value;
    key = it->first.as<string>();
    value = it->second.as<string>();
    std::cout << "Key: " << key << ", value: " << value << std::endl;
    weather_icon_map[key] = value;
  }

  MqttControllerConfig mqtt_config;
  mqtt_config.base_topic = mqtt_base_topic;
  mqtt_config.discovery_prefix = mqtt_discovery_prefix;
  mqtt_config.device_id = mqtt_device_id;
  mqtt_config.device_name = mqtt_device_name;
  mqtt_config.enable_discovery = mqtt_enable_discovery;
  mqtt_config.light_state_topic = light_state_topic;
  mqtt_config.light_command_topic = light_command_topic;
  mqtt_config.light_brightness_state_topic = light_brightness_state_topic;
  mqtt_config.light_brightness_command_topic =
      light_brightness_command_topic;
  ServiceRegistry services(spotify_refresh_token, spotify_client_id,
                           spotify_client_secret);
  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  const string bdf_8x13_font_path(base_path + "/fonts/8x13.bdf");
  const string bdf_5x7_font_path(base_path + "/fonts/5x7.bdf");

  rgb_matrix::Font main_font;
  if (!main_font.LoadFont(bdf_8x13_font_path.c_str())) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_8x13_font_path.c_str());
    return 1;
  }
  rgb_matrix::Font menu_font;
  if (!menu_font.LoadFont(bdf_5x7_font_path.c_str())) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_5x7_font_path.c_str());
    return 1;
  }

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  printf("CTRL-C for exit.\n");

  rgb_matrix::DrawText(offscreen_canvas, main_font, 4, 8 + main_font.baseline(),
                       color, nullptr, "Loading", letter_spacing);
  offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);

  const string image_path("/img");

  for (auto const &dir_entry :
       filesystem::directory_iterator{base_path + image_path}) {
    if (dir_entry.path().extension() == ".png") {
      ImageVector image =
          LoadImageAndScaleImage((dir_entry.path()).c_str(), 13, 13);

      cout << "Loading image: " << dir_entry.path() << endl;

      if (image.size() == 0) {
        cout << "FAILED TO LOAD IMAGE" << dir_entry.path() << endl;
      } else {
        state.image_map[dir_entry.path().stem()] = image[0];
      }
    } else {
      cout << "Found a non PNG file: " << dir_entry.path()
           << ", extension is: " << dir_entry.path().extension() << endl;
    }
  }

  for (auto const &dir_entry :
       filesystem::directory_iterator{base_path + image_path + "/weather"}) {
    if (dir_entry.path().extension() == ".png") {
      ImageVector image =
          LoadImageAndScaleImage((dir_entry.path()).c_str(), 13, 13);

      cout << "Loading weather image: " << dir_entry.path() << endl;

      if (image.size() == 0) {
        cout << "FAILED TO LOAD IMAGE" << dir_entry.path() << endl;
      } else {
        state.image_map[dir_entry.path().stem()] = image[0];
      }
    } else {
      cout << "Found a non PNG file: " << dir_entry.path()
           << ", extension is: " << dir_entry.path().extension() << endl;
    }
  }

  cout << "Images loaded: " << endl;
  for (const auto &[key, value] : state.image_map)
    cout << '[' << key << "] = " << value.constImageInfo() << endl;

  std::vector<LineType> default_line1_options = {LineType::Bus};
  std::vector<LineType> default_line2_options = {LineType::CurrentTime,
                                                 LineType::CurrentDate,
                                                 LineType::Weather};
  std::vector<LineType> line1_options = parseLineOptions(
      config["line1_options"], default_line1_options);
  std::vector<LineType> line2_options = parseLineOptions(
      config["line2_options"], default_line2_options);
  const int line1_rotate_seconds =
      getOptionalConfig<int>(config, "line1_rotate_seconds", 15);
  const int line2_rotate_seconds =
      getOptionalConfig<int>(config, "line2_rotate_seconds", 10);
  const int near_end_chars =
      getOptionalConfig<int>(config, "near_end_chars", 6);
  const int min_display_seconds =
      getOptionalConfig<int>(config, "min_display_seconds", 10);

  ScrollingLineScreenSettings scrollingLineScreenSettings =
      ScrollingLineScreenSettings(
          defaults.cols, defaults.rows, &main_font, color, bg_color,
          &state.speed, &state.mutex, std::move(line1_options),
          std::move(line2_options), std::chrono::seconds(line1_rotate_seconds),
          std::chrono::seconds(line2_rotate_seconds), letter_spacing,
          weather_api_key, near_end_chars,
          std::chrono::seconds(min_display_seconds));

  auto imageMapPtr = std::shared_ptr<std::map<std::string, Magick::Image>>(
      &state.image_map, [](void *) {});

  auto scrollingTwoLineScreen = std::make_unique<ScrollingLineScreen>(
      imageMapPtr, weather_icon_map, scrollingLineScreenSettings, &services);
  ScrollingLineScreen *scrolling_screen_ptr = scrollingTwoLineScreen.get();

  cout << "Setting up update thread" << endl;

  auto game_of_life_screen =
      std::make_unique<GameOfLifeScreen>(offscreen_canvas, 500, true);
  game_of_life_screen->set_hidden();

  auto rotating_box = std::make_unique<RotatingBoxScreen>(offscreen_canvas);
  rotating_box->set_hidden();

  std::vector<std::unique_ptr<Screen>> screens_owned;
  screens_owned.push_back(std::move(scrollingTwoLineScreen));
  screens_owned.push_back(std::move(game_of_life_screen));
  screens_owned.push_back(std::move(rotating_box));

  vector<Screen *> screens_to_render;
  for (auto &screen : screens_owned) {
    screens_to_render.push_back(screen.get());
  }

  vector<UpdatableScreen *> screens_to_update;

  screens_to_update.push_back(
      static_cast<UpdatableScreen *>(screens_to_render[0]));
  screens_to_update.push_back(
      static_cast<UpdatableScreen *>(screens_to_render[1]));

  std::vector<std::string> mqtt_topics;
  MQTTClient mqttClient(mqtt_server, mqtt_client_id, mqtt_user_name,
                        mqtt_password, mqtt_topics);
  MqttController mqtt_controller(&mqttClient, &state, scrolling_screen_ptr,
                                 mqtt_config);
  mqtt_topics = mqtt_controller.command_topics();
  mqttClient.update_topics(mqtt_topics);

  mqtt_controller.publishDiscovery();
  mqtt_controller.publishStartupState();

  cout << "Done publishing startup MQTT messages" << endl;

  thread handleMqttMessagesThread(&MqttController::run, &mqtt_controller,
                                  &running);

  thread updateThread(updateLines, screens_to_update, &running);

  MenuScreen menu =
      MenuScreen(letter_spacing, &menu_font, width, &state, &screens_to_render);

  offscreen_canvas->Clear();

  while (!interrupt_received) {
    bool screen_on = true;
    int current_brightness = 0;
    float current_speed = 0.0f;
    {
      std::lock_guard<std::mutex> lock(state.mutex);
      screen_on = state.screen_on;
      current_brightness = state.current_brightness;
      current_speed = state.speed;
    }

    if (screen_on) {
      offscreen_canvas->SetBrightness(current_brightness);
      offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);

      for (vector<Screen *>::iterator screen = screens_to_render.begin();
           screen != screens_to_render.end(); screen++) {
        (*screen)->render(offscreen_canvas);
      }
    } else {
      offscreen_canvas->SetBrightness(current_brightness);
      offscreen_canvas->Fill(0, 0, 0);
    }
    menu.render(offscreen_canvas);
    offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
    if (current_speed == 0) {
      usleep(1000000);
    } else {
      usleep(1000000 / current_speed / main_font.CharacterWidth('W'));
    }
  }

  running.store(false);
  mqttClient.stop_consuming();
  if (handleMqttMessagesThread.joinable()) {
    handleMqttMessagesThread.join();
  }
  if (updateThread.joinable()) {
    updateThread.join();
  }

  // Finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  return 0;
}

