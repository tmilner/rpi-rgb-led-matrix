#include "music-line.h"
#include "img_utils.h"
#include <climits>
#include <iostream>
#include <mutex>
using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

MusicLine::MusicLine(
    std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
    SpotifyClient spotifyClient, Radio6Client radio6Client,
    ScrollingLineSettings settings)
    : ScrollingLine(settings), name{std::string("Radio 6 Line")},
      spotifyClient(spotifyClient), radio6Client(radio6Client) {
  this->current_line = "Loading";
  this->image_map = image_map;
  this->image_key = "radio6icon";
  this->is_visible = true;
  auto now = std::chrono::system_clock::now();
  this->last_update = now - 5min;
}

Magick::Image *MusicLine::getIcon() {
  std::lock_guard<std::recursive_mutex> lock(line_mutex);
  return &(*this->image_map)[this->image_key];
}

std::string *MusicLine::getName() { return &this->name; }

void MusicLine::render(FrameCanvas *offscreen_canvas, char opacity) {
  if (!is_visible) {
    return;
  }
  if (opacity >= (CHAR_MAX / 2)) {
    this->renderLine(offscreen_canvas);
  }
  offscreen_canvas->SetPixels(0, this->y, 13, 16, 0, 0, 0);
  CopyImageToCanvas(this->getIcon(), offscreen_canvas, 0, this->y + 1, opacity);
  rgb_matrix::DrawLine(offscreen_canvas, 13, this->y, 13, this->y + 16,
                       Color(130, 100, 73));
}

void MusicLine::update() {
  const auto now = std::chrono::system_clock::now();

  if (((now - this->last_update) / 1s) < this->update_after_seconds) {
    return;
  }

  std::cout << "Checking Spotify" << std::endl;

  bool showSpotify = true;

  try {
    std::optional<SpotifyClient::NowPlaying> nowPlaying =
        this->spotifyClient.getNowPlaying();

    if (nowPlaying.has_value() && nowPlaying.value().is_playing) {
      std::cout << "Spotify now playing " << nowPlaying.value().track_name
                << std::endl;

      {
        std::lock_guard<std::recursive_mutex> lock(line_mutex);
        this->last_update = now;
        this->image_key = "spotify";
        this->current_line.clear();
        this->current_line.append(nowPlaying.value().artist)
            .append(" - ")
            .append(nowPlaying.value().track_name);
      }
    } else {
      showSpotify = false;
    }
  } catch (std::runtime_error &e) {
    showSpotify = false;
    std::cerr << "Exception when updating music line" << e.what() << std::endl;
  }

  if (!showSpotify) {
    try {
      // TODO move to a client for radio 6 like spotify...
      std::cout << "Fetching radio6 data from " << this->radio6_url
                << std::endl;

      Radio6Client::NowPlaying nowPlaying = this->radio6Client.getNowPlaying();

      {
        std::lock_guard<std::recursive_mutex> lock(line_mutex);
        this->last_update = now;
        this->image_key = "radio6icon";
        this->current_line.clear();
        this->current_line.append(nowPlaying.artist)
            .append(" - ")
            .append(nowPlaying.track_name);
      }
    } catch (std::runtime_error &e) {
      std::cerr << "Exception when updating music line" << e.what()
                << std::endl;
    }
  }
}
