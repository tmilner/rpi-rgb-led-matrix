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

#include <curl/curl.h>

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <json/json.h>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
  interrupt_received = true;
}

static int usage(const char *progname)
{
  fprintf(stderr, "usage: %s [options] <text>\n", progname);
  fprintf(stderr, "Takes text and scrolls it with speed -s\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr,
          "\t-s <speed>        : Approximate letters per second. "
          "(Zero for no scrolling)\n"
          "\t-l <loop-count>   : Number of loops through the text. "
          "-1 for endless (default)\n"
          "\t-f <font-file>    : Use given font.\n"
          "\t-x <x-origin>     : X-Origin of displaying text (Default: 0)\n"
          "\t-y <y-origin>     : Y-Origin of displaying text (Default: 0)\n"
          "\t-t <track=spacing>: Spacing pixels between letters (Default: 0)\n"
          "\n"
          "\t-C <r,g,b>        : Text-Color. Default 255,255,0\n"
          "\t-B <r,g,b>        : Background-Color. Default 0,0,0\n"
          "\n");
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

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

int main(int argc, char *argv[])
{
  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "adafruit-hat"; // or e.g. "adafruit-hat"
  defaults.rows = 32;
  defaults.cols = 64;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = true;

  RGBMatrix *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);

  const std::string url("https://nowplaying.jameswragg.com/api/bbc6music");

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
      std::cout << "Successfully parsed JSON data" << std::endl;
      std::cout << "\nJSON data received:" << std::endl;
      std::cout << jsonData.toStyledString() << std::endl;

      const std::string artist(jsonData["tracks"][0]["artist"].asString());

      const std::string track_name(jsonData["tracks"][0]["name"].asString());

      std::cout << "\tArtist: " << artist << std::endl;
      std::cout << "\tTrack Name: " << track_name << std::endl;
      std::cout << std::endl;
    }
    else
    {
      std::cout << "Could not parse HTTP data as JSON" << std::endl;
      std::cout << "HTTP data was:\n"
                << *httpData.get() << std::endl;
      return 1;
    }
  }
  else
  {
    std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;
    return 1;
  }

  // RGBMatrix::Options matrix_options;
  // rgb_matrix::RuntimeOptions runtime_opt;
  // if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
  //                                        &matrix_options, &runtime_opt)) {
  //   return usage(argv[0]);
  // }

  Color color(255, 255, 0);
  Color bg_color(0, 0, 0);

  const char *bdf_font_file = NULL;
  std::string line;
  /* x_origin is set by default just right of the screen */
  const int x_default_start = (defaults.chain_length * defaults.cols) + 5;
  int x_orig = x_default_start;
  int y_orig = 0;
  int letter_spacing = 0;
  float speed = 7.0f;
  int loops = -1;

  int opt;
  while ((opt = getopt(argc, argv, "x:y:f:C:B:t:s:l:")) != -1)
  {
    switch (opt)
    {
    case 's':
      speed = atof(optarg);
      break;
    case 'l':
      loops = atoi(optarg);
      break;
    case 'x':
      x_orig = atoi(optarg);
      break;
    case 'y':
      y_orig = atoi(optarg);
      break;
    case 'f':
      bdf_font_file = strdup(optarg);
      break;
    case 't':
      letter_spacing = atoi(optarg);
      break;
    case 'C':
      if (!parseColor(&color, optarg))
      {
        fprintf(stderr, "Invalid color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    case 'B':
      if (!parseColor(&bg_color, optarg))
      {
        fprintf(stderr, "Invalid background color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    default:
      return usage(argv[0]);
    }
  }

  for (int i = optind; i < argc; ++i)
  {
    line.append(argv[i]).append(" ");
  }

  if (line.empty())
  {
    fprintf(stderr, "Add the text you want to print on the command-line.\n");
    return usage(argv[0]);
  }

  if (bdf_font_file == NULL)
  {
    fprintf(stderr, "Need to specify BDF font-file with -f\n");
    return usage(argv[0]);
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

  const bool all_extreme_colors = (matrix_options.brightness == 100) && FullSaturation(color) && FullSaturation(bg_color);
  if (all_extreme_colors)
    canvas->SetPWMBits(1);

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
  else if (x_orig == x_default_start)
  {
    // There would be no scrolling, so text would never appear. Move to front.
    x_orig = 0;
  }

  int x = x_orig;
  int y = y_orig;
  int length = 0;

  while (!interrupt_received && loops != 0)
  {
    offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
    // length = holds how many pixels our text takes up
    length = rgb_matrix::DrawText(offscreen_canvas, font,
                                  x, y + font.baseline(),
                                  color, nullptr,
                                  line.c_str(), letter_spacing);

    if (speed > 0 && --x + length < 0)
    {
      x = x_orig;
      if (loops > 0)
        --loops;
    }

    // Swap the offscreen_canvas with canvas on vsync, avoids flickering
    offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);
    usleep(delay_speed_usec);
  }

  // Finished. Shut down the RGB matrix.
  delete canvas;

  return 0;
}
