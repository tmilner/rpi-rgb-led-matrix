#ifndef RADIO6_LINE_UPDATER_H
#define RADIO6_LINE_UPDATER_H
#include "radio6-client.h"
#include "scrolling-line.h"
#include "spotify-client.h"
#include "updateable-screen.h"
#include <Magick++.h>
#include <chrono>

class MusicLine : public UpdateableScreen, public ScrollingLine {
public:
  MusicLine(std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
            SpotifyClient *spotifyClient, Radio6Client *radio6Client,
            ScrollingLineSettings settings);
  void update();
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  Magick::Image *getIcon();
  SpotifyClient *spotifyClient;
  Radio6Client *radio6Client;
  std::string radio6_url;
  std::chrono::time_point<std::chrono::system_clock> last_update;
  static const int update_after_seconds = 20;
  std::shared_ptr<std::map<std::string, Magick::Image>> image_map;
  std::string image_key;
  std::string name;
};
#endif /*RADIO6_LINE_UPDATER_H*/
