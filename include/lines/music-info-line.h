#ifndef MUSIC_INFO_LINE_H
#define MUSIC_INFO_LINE_H
#include "clients/radio6-client.h"
#include "clients/spotify-client.h"
#include "lines/scrolling-line.h"
#include "screens/updatable-screen.h"
#include <Magick++.h>
#include <chrono>

class MusicInfoLine : public UpdatableScreen, public ScrollingLine {
public:
  MusicInfoLine(std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
                SpotifyClient *spotifyClient, Radio6Client *radio6Client,
                ScrollingLineSettings settings);
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  Magick::Image *getIcon();
  void applyPendingIconIfReady();
  SpotifyClient *spotifyClient;
  Radio6Client *radio6Client;
  std::string radio6_url;
  std::chrono::time_point<std::chrono::system_clock> last_update;
  static const int update_after_seconds = 20;
  std::shared_ptr<std::map<std::string, Magick::Image>> image_map;
  std::string image_key;
  std::string pending_image_key;
  std::string name;
};
#endif /*MUSIC_INFO_LINE_H*/
