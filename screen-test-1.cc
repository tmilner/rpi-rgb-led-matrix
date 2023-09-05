// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to scroll text.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

// For a utility with a few more features see
// ../utils/text-scroller.cc

#include "led-matrix.h"
#include "graphics.h"

#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <filesystem>
#include <stdexcept>

#include <curl/curl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <json/json.h>
#include <screen-line.h>

#include <Magick++.h>
#include <magick/image.h>

namespace fs = std::filesystem;

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
  interrupt_received = true;
}

std::string line1str = "Loading";
std::string line2str = "Loading";

static bool parseColor(Color *c, const char *str)
{
  return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
}

static bool FullSaturation(const Color &c)
{
  return (c.r == 0 || c.r == 255) && (c.g == 0 || c.g == 255) && (c.b == 0 || c.b == 255);
}

namespace
{
  std::size_t callback(
      const char *in,
      std::size_t size,
      std::size_t num,
      std::string *out)
  {
    const std::size_t totalBytes(size * num);
    out->append(in, totalBytes);
    return totalBytes;
  }
}

using ImageVector = std::vector<Magick::Image>;

// Given the filename, load the image and scale to the size of the
// matrix.
// // If this is an animated image, the resutlting vector will contain multiple.
static ImageVector LoadImageAndScaleImage(const char *filename,
                                          int target_width,
                                          int target_height)
{
  ImageVector result;

  ImageVector frames;
  try
  {
    readImages(&frames, filename);
  }
  catch (std::exception &e)
  {
    if (e.what())
      fprintf(stderr, "%s\n", e.what());
    return result;
  }

  if (frames.empty())
  {
    fprintf(stderr, "No image found.");
    return result;
  }

  // Animated images have partial frames that need to be put together
  if (frames.size() > 1)
  {
    Magick::coalesceImages(&result, frames.begin(), frames.end());
  }
  else
  {
    result.push_back(frames[0]); // just a single still image.
  }

  for (Magick::Image &image : result)
  {
    image.scale(Magick::Geometry(target_width, target_height));
  }

  return result;
}

// Copy an image to a Canvas. Note, the RGBMatrix is implementing the Canvas
// interface as well as the FrameCanvas we use in the double-buffering of the
// animted image.
void CopyImageToCanvas(const Magick::Image &image, Canvas *canvas, int offset_x, int offset_y)
{
  // Copy all the pixels to the canvas.
  for (size_t y = 0; y < image.rows(); ++y)
  {
    for (size_t x = 0; x < image.columns(); ++x)
    {
      const Magick::Color &c = image.pixelColor(x, y);
      if (c.alphaQuantum() < 256)
      {
        canvas->SetPixel(x + offset_x, y + offset_y,
                         ScaleQuantumToChar(c.redQuantum()),
                         ScaleQuantumToChar(c.greenQuantum()),
                         ScaleQuantumToChar(c.blueQuantum()));
      }
    }
  }
}

Json::Value callJsonAPI(std::string url)
{
  CURL *curl = curl_easy_init();

  // Set remote URL.
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  // Don't bother trying IPv6, which would increase DNS resolution time.
  curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
  // Don't wait forever, time out after 10 seconds.
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
  // Follow HTTP redirects if necessary.
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  // Response information.
  int httpCode(0);
  std::unique_ptr<std::string> httpData(new std::string());

  // Hook up data handling function.
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
  // Hook up data container (will be passed as the last parameter to the
  // callback handling function).  Can be any pointer type, since it will
  // internally be passed as a void pointer.
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

  // Run our HTTP GET command, capture the HTTP response code, and clean up.
  curl_easy_perform(curl);
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
  curl_easy_cleanup(curl);

  if (httpCode == 200)
  {
    std::cout << "\nGot successful response from " << url << std::endl;

    // Response looks good - done using Curl now.  Try to parse the results
    // and print them out.
    Json::Value jsonData;
    Json::Reader jsonReader;

    if (jsonReader.parse(*httpData, jsonData))
    {
      return jsonData;
    }
    else
    {
      std::cout << "Could not parse HTTP data as JSON" << std::endl;
      std::cout << "HTTP data was:\n"
                << *httpData.get() << std::endl;
      throw std::runtime_error("Failed to Parse JSON");
    }
  }
  else
  {
    std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;
    throw std::runtime_error("Got non 200 Response from URL");
  }
}

bool updateWeatherData(std::string *line)
{
  const std::string url("https://api.openweathermap.org/data/2.5/weather?lon=-0.093014&lat=51.474087&appid=9b8d4c21eb390f8f70f34a5705ed1546");

  try
  {
    Json::Value jsonData = callJsonAPI(url);

    if (jsonData != NULL)
    {
      const std::string condition(jsonData["weather"][0]["description"].asString());

      const std::string temp(jsonData["mian"]["temp"].asString());

      std::cout << "\tCondition: " << condition << std::endl;
      std::cout << "\tTempt: " << temp << std::endl;
      std::cout << std::endl;

      line->clear();
      line->append(condition).append(" - ").append(temp);
      return true;
    }
  }
  catch (std::runtime_error &e)
  {
    return false;
  }
}

bool updateRadio6Data(std::string *line)
{
  const std::string url("https://nowplaying.jameswragg.com/api/bbc6music");

  try
  {
    Json::Value jsonData = callJsonAPI(url);

    if (jsonData != NULL)
    {
      const std::string artist(jsonData["tracks"][0]["artist"].asString());

      const std::string track_name(jsonData["tracks"][0]["name"].asString());

      std::cout << "\tArtist: " << artist << std::endl;
      std::cout << "\tTrack Name: " << track_name << std::endl;
      std::cout << std::endl;

      line->clear();
      line->append(artist).append(" - ").append(track_name);
      return true;
    }
  }
  catch (std::runtime_error &e)
  {
    return false;
  }
}

using namespace std::literals::chrono_literals;
void radio6UpdateLoop(std::string *line)
{
  while (true)
  {
    updateRadio6Data(line);
    std::this_thread::sleep_for(30s);
  }
}

void weatherUpdateLoop(std::string *line)
{
  while (true)
  {
    updateWeatherData(line);
    std::this_thread::sleep_for(360s);
  }
}

int main(int argc, char *argv[])
{

  Magick::InitializeMagick(*argv);

  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "adafruit-hat-pwm";
  defaults.rows = 32;
  defaults.cols = 64;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = false;
  defaults.limit_refresh_rate_hz = 25;

  RGBMatrix *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);

  Color color(255, 255, 0);
  Color red(214, 40, 40);

  Color bg_color(0, 0, 0);

  const char *bdf_font_file = "fonts/8x13.bdf";
  /* x_origin is set by default just right of the screen */
  const int x_default_start = (defaults.chain_length * defaults.cols) + 5;

  int letter_spacing = 0;
  float speed = 3.0f;
  const char *base_path_c = ".";

  int opt;
  while ((opt = getopt(argc, argv, "f:s:p:")) != -1)
  {
    switch (opt)
    {
    case 's':
      speed = atof(optarg);
      break;
    case 'f':
      bdf_font_file = strdup(optarg);
      break;
    case 'p':
      base_path_c = strdup(optarg);
      break;
    }
  }

  const std::string base_path(base_path_c);

  std::thread radio6Thread(radio6UpdateLoop, &line1str);
  std::thread weatherThread(weatherUpdateLoop, &line2str);

  if (bdf_font_file == NULL)
  {
    fprintf(stderr, "Need to specify BDF font-file with -f\n");
    return -1;
  }

  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font_file))
  {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return 1;
  }

  /* RGBMatrix *canvas = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt); */
  if (canvas == NULL)
    return 1;

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  printf("CTRL-C for exit.\n");

  // Create a new canvas to be used with led_matrix_swap_on_vsync
  FrameCanvas *offscreen_canvas = canvas->CreateFrameCanvas();

  int delay_speed_usec = 1000000;
  if (speed > 0)
  {
    delay_speed_usec = 1000000 / speed / font.CharacterWidth('W');
  }

  const std::string radio6ImagePath("/img/bbcradio6musicsmall.png");

  ImageVector radio6Image = LoadImageAndScaleImage(
      (base_path + radio6ImagePath).c_str(),
      13,
      13);
  if (radio6Image.size() == 0)
  {
    std::cout << "FAILED TO LOAD RADIO 6 IMAGE" << std::endl;
  }

  const std::string weatherImagePath("/img/cloudysun.png");

  ImageVector weatherImage = LoadImageAndScaleImage(
      (base_path + weatherImagePath).c_str(),
      13,
      13);
  if (weatherImage.size() == 0)
  {
    std::cout << "FAILED TO LOAD WEATHER IMAGE" << std::endl;
  }

  ScreenLine line1(
      speed,
      x_default_start,
      1,
      letter_spacing,
      &font,
      color,
      &line1str);

  ScreenLine line2(
      speed,
      x_default_start,
      18,
      letter_spacing,
      &font,
      color,
      &line2str);

  while (!interrupt_received)
  {
    offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);

    line1.render(offscreen_canvas);
    line2.render(offscreen_canvas);
    line1.updateText(&line1str);
    line2.updateText(&line2str);

    offscreen_canvas->SetPixels(0, 0, 13, 32, 0, 0, 0);

    rgb_matrix::DrawLine(offscreen_canvas, 14, 0, 14, 32, red);

    CopyImageToCanvas(radio6Image[0], offscreen_canvas, 0, 1);

    CopyImageToCanvas(weatherImage[0], offscreen_canvas, 0, 18);

    // Swap the offscreen_canvas with canvas on vsync, avoids flickering
    offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);
    usleep(delay_speed_usec);
  }

  // Finished. Shut down the RGB matrix.
  delete canvas;

  return 0;
}
