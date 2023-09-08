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
#include <iomanip>
#include <sstream>

#include <yaml-cpp/yaml.h>
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

std::map<std::string, Magick::Image> image_map{};

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
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
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
  CURLcode code = curl_easy_perform(curl);
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
      httpData.release();
      return jsonData;
    }
    else
    {
      std::cout << "Could not parse HTTP data as JSON" << std::endl;
      std::cout << "HTTP data was:\n"
                << *httpData.get() << std::endl;
      httpData.release();

      throw std::runtime_error("Failed to Parse JSON");
    }
  }
  else
  {
    httpData.release();
    std::cout << "Non 200 from " << url << ". Code = " << httpCode << " Curl code " << curl_easy_strerror(code) << std::endl;
    throw std::runtime_error("Got non 200 Response from URL");
  }
}

void updateRadio6(std::string *line)
{
  const std::string url("https://nowplaying.jameswragg.com/api/bbc6music?limit=1");
  std::cout << "Fetching radio6 data from " << url << std::endl;

  try
  {
    Json::Value jsonData = callJsonAPI(url);

    const std::string artist(jsonData["tracks"][0]["artist"].asString());

    const std::string track_name(jsonData["tracks"][0]["name"].asString());

    std::cout << "\tArtist: " << artist << std::endl;
    std::cout << "\tTrack Name: " << track_name << std::endl;
    std::cout << std::endl;

    line->clear();
    line->append(artist).append(" - ").append(track_name);
  }
  catch (std::runtime_error &e)
  {
    printf("Failed to fetch weather\n");
  }
}

const std::string weather_base_url("https://api.openweathermap.org/data/2.5/weather?lon=-0.093014&lat=51.474087&appid=");
void updateWeather(std::string *line, const std::string weather_api_key, Magick::Image *current_image)
{
  const std::string url = weather_base_url + weather_api_key;
  std::cout << "Fetching wether data from " << url << std::endl;
  try
  {
    Json::Value jsonData = callJsonAPI(url);

    const std::string condition(jsonData["weather"][0]["description"].asString());
    const std::string weather_icon(jsonData["weather"][0]["icon"].asString());
    const double kevinScale = 273.15;
    const double temp = jsonData["main"]["temp"].asDouble() - kevinScale;

    std::stringstream temp_str_stream;
    temp_str_stream << std::fixed << std::setprecision(1) << temp;
    std::string temp_str = temp_str_stream.str();

    std::cout << "\tCondition: " << condition << std::endl;
    std::cout << "\tTemp: " << temp << std::endl;    
    std::cout << "\tIcon: " << weather_icon << std::endl;

    std::cout << std::endl;

    current_image = &(image_map.at(weather_icon));
    line->clear();
    line->append(temp_str).append("℃");
  }
  catch (std::runtime_error &e)
  {
    printf("Failed to fetch radio6\n");
  }
}

using namespace std::literals::chrono_literals;
void updateLines(std::string *line1, std::string *line2, const std::string weather_api_key, Magick::Image *current_image)
{
  int refreshCount = 0;

  while (true)
  {
    updateRadio6(line1);
    if (refreshCount % 10 == 0)
    {
      updateWeather(line2, weather_api_key, current_image);
    }
    std::this_thread::sleep_for(30s);
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

  RGBMatrix *matrix = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);

  Color color(240, 160, 100);
  Color red(233, 110, 80);

  Color bg_color(0, 0, 0);

  const char *bdf_font_file = "fonts/8x13.bdf";
  /* x_origin is set by default just right of the screen */
  const int x_default_start = (defaults.chain_length * defaults.cols) + 2;

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

  // Weather API Key
  YAML::Node config = YAML::LoadFile(base_path + "/config.yaml");
  const std::string weather_api_key = config["weather_api_key"].as<std::string>();

  // Weather String
  //  YAML::Node weather_file = YAML::LoadFile(base_path + "/weather_strings.yaml");
  //  YAML::Node weather_strings = weather_file["strings"];

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
  if (matrix == NULL)
    return 1;

  // Let's request all input bits and see which are actually available.
  // This will differ depending on which hardware mapping you use and how
  // many parallel chains you have.
  const uint64_t available_inputs = matrix->RequestInputs(0xffffffff);
  fprintf(stderr, "Available GPIO-bits: ");
  for (int b = 0; b < 32; ++b)
  {
    if (available_inputs & (1 << b))
      fprintf(stderr, "%d ", b);
  }
  fprintf(stderr, "\n");

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  printf("CTRL-C for exit.\n");

  // Create a new canvas to be used with led_matrix_swap_on_vsync
  FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();

  int delay_speed_usec = 1000000;
  if (speed > 0)
  {
    delay_speed_usec = 1000000 / speed / font.CharacterWidth('W');
  }

  const std::string radio6ImagePath("/img/radio6icon.png");

  std::cout << "Loading Radio6 Image" << std::endl;
  ImageVector radio6Image = LoadImageAndScaleImage(
      (base_path + radio6ImagePath).c_str(),
      11,
      11);

  if (radio6Image.size() == 0)
  {
    std::cout << "FAILED TO LOAD RADIO 6 IMAGE" << std::endl;
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
        image_map[dir_entry.path().stem()] = image[0];
      }
    }
    else
    {
      std::cout << "Found a non PNG file: " << dir_entry.path() << ", extension is: " << dir_entry.path().extension() << std::endl;
    }
  }

  Magick::Image *current_image = & (image_map.at("01d"));

  std::thread updateThread(updateLines, &line1str, &line2str, weather_api_key, current_image);

  ScreenLine line1(
      speed,
      1,
      letter_spacing,
      &font,
      color,
      &line1str,
      defaults.chain_length * defaults.cols,
      14);

  ScreenLine line2(
      speed,
      18,
      letter_spacing,
      &font,
      color,
      &line2str,
      defaults.chain_length * defaults.cols,
      14);

  while (!interrupt_received)
  {
    offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);

    line1.render(offscreen_canvas);
    line2.render(offscreen_canvas);
    line1.updateText(&line1str);
    line2.updateText(&line2str);

    offscreen_canvas->SetPixels(0, 0, 13, 32, 0, 0, 0);

    rgb_matrix::DrawLine(offscreen_canvas, 13, 0, 13, 32, red);

    CopyImageToCanvas(radio6Image[0], offscreen_canvas, 1, 2);

    CopyImageToCanvas(*current_image, offscreen_canvas, 0, 18);

    // Swap the offscreen_canvas with canvas on vsync, avoids flickering
    offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
    usleep(delay_speed_usec);
  }

  // Finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  return 0;
}
