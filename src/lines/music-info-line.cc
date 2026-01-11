#include "lines/music-info-line.h"
#include "core/img_utils.h"
#include <chrono>
#include <climits>
#include <iostream>
#include <mutex>
using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

MusicInfoLine::MusicInfoLine(
    std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
    SpotifyClient *spotifyClient, Radio6Client *radio6Client,
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

Magick::Image *MusicInfoLine::getIcon() {
  std::lock_guard<std::recursive_mutex> lock(line_mutex);
  return &(*this->image_map)[this->image_key];
}

std::string *MusicInfoLine::getName() { return &this->name; }

void MusicInfoLine::applyPendingIconIfReady() {
  std::lock_guard<std::recursive_mutex> lock(line_mutex);
  if (!pending_image_key.empty() && isReadyForUpdate()) {
    image_key = pending_image_key;
    pending_image_key.clear();
  }
}

void MusicInfoLine::render(FrameCanvas *offscreen_canvas, char opacity) {
  if (!is_visible) {
    return;
  }
  applyPendingIconIfReady();
  if (opacity >= (CHAR_MAX / 2)) {
    this->renderLine(offscreen_canvas);
  }
  offscreen_canvas->SetPixels(0, this->y, 13, 16, 0, 0, 0);
  CopyImageToCanvas(this->getIcon(), offscreen_canvas, 0, this->y + 1, opacity);
  rgb_matrix::DrawLine(offscreen_canvas, 13, this->y, 13, this->y + 16,
                       Color(130, 100, 73));
}

void MusicInfoLine::update() {
  const auto now = std::chrono::system_clock::now();

  if (((now - this->last_update) / 1s) < this->update_after_seconds) {
    return;
  }
  if (!shouldFetchUpdate()) {
    return;
  }

  std::cout << "Checking Spotify" << std::endl;

  bool showSpotify = true;

  try {
    std::optional<SpotifyClient::NowPlaying> nowPlaying =
        this->spotifyClient->getNowPlaying();

    if (nowPlaying.has_value() && nowPlaying.value().is_playing) {
      std::cout << "Spotify now playing " << nowPlaying.value().track_name
                << std::endl;

      std::string new_line = nowPlaying.value().artist + " - " +
                             nowPlaying.value().track_name;
      this->last_update = now;
      if (isReadyForUpdate()) {
        std::lock_guard<std::recursive_mutex> lock(line_mutex);
        this->image_key = "spotify";
      } else {
        std::lock_guard<std::recursive_mutex> lock(line_mutex);
        this->pending_image_key = "spotify";
      }
      updateText(&new_line);
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

      Radio6Client::NowPlaying nowPlaying =
          this->radio6Client->getNowPlaying();

      std::string new_line =
          nowPlaying.artist + " - " + nowPlaying.track_name;
      this->last_update = now;
      if (isReadyForUpdate()) {
        std::lock_guard<std::recursive_mutex> lock(line_mutex);
        this->image_key = "radio6icon";
      } else {
        std::lock_guard<std::recursive_mutex> lock(line_mutex);
        this->pending_image_key = "radio6icon";
      }
      updateText(&new_line);
    } catch (std::runtime_error &e) {
      std::cerr << "Exception when updating music line" << e.what()
                << std::endl;
    }
  }
}
