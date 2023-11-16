#ifndef RADIO6_LINE_UPDATER_H
#define RADIO6_LINE_UPDATER_H
#include "updateable-screen.h"
#include "scrolling-line.h"
#include "json-fetcher.h"
#include <Magick++.h>
#include <chrono>
#include "spotify_client.h"

class MusicLine : public UpdateableScreen, ScrollingLine
{
public:
    MusicLine(std::map<std::string, Magick::Image> *image_map, SpotifyClient spotifyClient, ScrollingLineSettings settings);
    void update();
    void render(FrameCanvas *offscreen_canvas);
    std::string *getName();

private:
    Magick::Image *getIcon();
    SpotifyClient spotifyClient;
    JSONFetcher *fetcher;
    std::string radio6_url;
    std::chrono::time_point<std::chrono::system_clock> last_update;
    static const int update_after_seconds = 20;
    std::map<std::string, Magick::Image> *image_map{};
    std::string image_key;
    std::string name;
};
#endif /*RADIO6_LINE_UPDATER_H*/