// -*- mode: c++; c-basic-offset: 2; indent-t
// Small example how to scroll text.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

// For a utility with a few more features see
// ../utils/text-scroller.cc

#include "game-of-life.h"
#include "graphics.h"
#include "img_utils.h"
#include "led-matrix.h"
#include "mqtt-client.h"
#include "radio6-client.h"
#include "rotating-box.h"
#include "screen-menu.h"
#include "screen_state.h"
#include "scrolling-line-screen.h"
#include "spotify-client.h"
#include "tfl-client.h"
#include "updateable-screen.h"

#include <iostream>
#include <string>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>

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

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) { interrupt_received = true; }

using namespace literals::chrono_literals;
void updateLines(vector<UpdateableScreen *> screens_to_update) {
  while (true) {
    for (vector<UpdateableScreen *>::iterator screen =
             screens_to_update.begin();
         screen != screens_to_update.end(); screen++) {
      (*screen)->update();
    }
    usleep(2 * 1000 * 1000);
  }
}

void handleMQTTMessages(MQTTClient *mqttClient, ScreenState *state,
                        std::string light_brightness_command_topic,
                        std::string light_brightness_state_topic,
                        std::string light_state_topic,
                        std::string light_command_topic) {
  while (true) {
    try {
      auto message = mqttClient->consume_message();
      if (!message) {
        cerr << "Failed to consume message!" << endl;
        break;
      }
      std::cout << "Recieved message for topic " << message->get_topic()
                << " with payload " << message->to_string() << endl;

      if (message->get_topic() == light_brightness_command_topic) {
        std::cout << "Updating brightness!" << message->to_string() << endl;
        std::string message_contents = message->to_string();
        int new_brightness = stoi(message_contents);
        if (new_brightness >= 0 && new_brightness <= 100) {
          state->current_brightness = new_brightness;
          mqtt::message_ptr brightnessMessage = mqtt::make_message(
              light_brightness_state_topic, to_string(new_brightness));
          brightnessMessage->set_qos(QOS);
          brightnessMessage->set_retained(true);
          mqttClient->publish_message(brightnessMessage);
          std::cout << "Brightness updated via MQTT to " << new_brightness
                    << endl;
        }
      } else if (message->get_topic() == light_command_topic) {
        std::cout << "Updating state!" << message->to_string() << endl;
        std::string message_contents = message->to_string();
        if (message_contents == "ON") {
          state->screen_on = true;
          mqtt::message_ptr stateMessage =
              mqtt::make_message(light_state_topic, "ON");
          stateMessage->set_qos(QOS);
          stateMessage->set_retained(true);
          mqttClient->publish_message(stateMessage);
          std::cout << "State updated via MQTT to ON" << endl;
        } else if (message_contents == "OFF") {
          state->screen_on = false;
          mqtt::message_ptr stateMessage =
              mqtt::make_message(light_state_topic, "OFF");
          stateMessage->set_qos(QOS);
          stateMessage->set_retained(true);
          mqttClient->publish_message(stateMessage);
          std::cout << "State updated via MQTT to OFF" << endl;
        }
      }
    } catch (const mqtt::exception &exc) {
      cerr << "Exception on MQTT handler thread" << exc << endl;
    } catch (exception e) {
      cerr << "generic exception on MQTT handler thread" << endl;
    }
  }
}

int main(int argc, char *argv[]) {
  ScreenState state;
  state.image_map = {};
  state.current_mode = ScreenMode::scrolling_lines;
  state.current_brightness = 100;
  state.speed = 1.5f;

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
  //  float speed = 1.0f;
  const char *base_path_c = ".";

  int opt;
  while ((opt = getopt(argc, argv, "p:")) != -1) {
    switch (opt) {
    case 'p':
      base_path_c = strdup(optarg);
      break;
    }
  }

  const string base_path(base_path_c);

  YAML::Node config = YAML::LoadFile(base_path + "/config.yaml");

  const string weather_api_key = config["weather_api_key"].as<string>();

  const string spotify_client_id = config["spotify_client_id"].as<string>();
  const string spotify_client_secret =
      config["spotify_client_secret"].as<string>();
  const string spotify_refresh_token =
      config["spotify_refresh_token"].as<string>();

  const string mqtt_server = config["mqtt_server"].as<string>();
  const string mqtt_user_name = config["mqtt_user_name"].as<string>();
  const string mqtt_password = config["mqtt_password"].as<string>();
  const string mqtt_client_id = config["mqtt_client_id"].as<string>();
  const string light_state_topic = config["light_state_topic"].as<string>();
  const string light_command_topic = config["light_command_topic"].as<string>();
  const string light_brightness_state_topic =
      config["light_brightness_state_topic"].as<string>();
  const string light_brightness_command_topic =
      config["light_brightness_command_topic"].as<string>();

  vector<string> topics;
  // topics.push_back(light_state_topic);
  topics.push_back(light_command_topic);
  // topics.push_back(light_brightness_state_topic);
  topics.push_back(light_brightness_command_topic);

  MQTTClient mqttClient(mqtt_server, mqtt_client_id, mqtt_user_name,
                        mqtt_password, topics);
  SpotifyClient spotifyClient(spotify_refresh_token, spotify_client_id,
                              spotify_client_secret);
  Radio6Client radio6Client;
  TflClient tflClient;

  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  const string bdf_8x13_font_path(base_path + "/fonts/8x13.bdf");
  const string bdf_5x7_font_path(base_path + "/fonts/5x7.bdf");

  rgb_matrix::Font main_font;
  if (!main_font.LoadFont(bdf_8x13_font_path.c_str())) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_8x13_font_path);
    return 1;
  }
  rgb_matrix::Font menu_font;
  if (!menu_font.LoadFont(bdf_5x7_font_path.c_str())) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_5x7_font_path);
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

  cout << "Images loaded: " << endl;
  for (const auto &[key, value] : state.image_map)
    cout << '[' << key << "] = " << value.constImageInfo() << endl;

  string current_image = "01d";

  cout << "Publishing ON message to MQTT" << endl;
  mqtt::message_ptr onMessage = mqtt::make_message(light_state_topic, "ON");
  onMessage->set_qos(1);
  onMessage->set_retained(true);
  mqttClient.publish_message(onMessage);

  cout << "Publishing Brightness message to MQTT" << endl;
  mqtt::message_ptr brightnessMessage = mqtt::make_message(
      light_brightness_state_topic, to_string(state.current_brightness));
  brightnessMessage->set_qos(1);
  brightnessMessage->set_retained(true);
  mqttClient.publish_message(brightnessMessage);

  cout << "Done publishing startup MQTT messages" << endl;

  thread handleMqttMessagesThread(
      handleMQTTMessages, &mqttClient, &state, light_brightness_command_topic,
      light_brightness_state_topic, light_state_topic, light_command_topic);

  ScrollingLineScreenSettings scrollingLineScreenSettings =
      ScrollingLineScreenSettings(
          defaults.cols, defaults.rows, &main_font, color, bg_color,
          &state.speed, letter_spacing, ScreenLineOption::radio6,
          ScreenLineOption::timeDateWeather, weather_api_key);

  ScrollingLineScreen *srollingTwoLineScreen =
      new ScrollingLineScreen(&state.image_map, scrollingLineScreenSettings,
                              spotifyClient, radio6Client, tflClient);

  cout << "Setting up update thread" << endl;

  GameOfLfeScreen *game_of_life_screen =
      new GameOfLfeScreen(offscreen_canvas, 500, true);
  game_of_life_screen->set_hidden();

  RotatingBox *rotating_box = new RotatingBox(offscreen_canvas);
  rotating_box->set_hidden();

  vector<Screen *> screens_to_render;

  screens_to_render.push_back(srollingTwoLineScreen);
  screens_to_render.push_back(game_of_life_screen);
  screens_to_render.push_back(rotating_box);

  vector<UpdateableScreen *> screens_to_update;

  screens_to_update.push_back(srollingTwoLineScreen);
  screens_to_update.push_back(game_of_life_screen);

  thread updateThread(updateLines, screens_to_update);

  ScreenMenu menu =
      ScreenMenu(letter_spacing, &menu_font, width, &state, &push_ok, &push_up,
                 &push_down, &screens_to_render);

  offscreen_canvas->Clear();

  while (!interrupt_received) {
    if (state.screen_on) {
      offscreen_canvas->SetBrightness(state.current_brightness);
      offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);

      for (vector<Screen *>::iterator screen = screens_to_render.begin();
           screen != screens_to_render.end(); screen++) {
        (*screen)->render(offscreen_canvas);
      }

      menu.render(offscreen_canvas);
      offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
      if (state.speed == 0) {
        usleep(1000000);
      } else {
        usleep(1000000 / state.speed / main_font.CharacterWidth('W'));
      }
    } else {
      offscreen_canvas->SetBrightness(state.current_brightness);
      offscreen_canvas->Fill(0, 0, 0);
      usleep(1000000000);
    }
  }

  // Finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  return 0;
}
