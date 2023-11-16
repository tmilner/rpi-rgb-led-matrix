#ifndef SPOTIFY_LIB_H
#define SPOTIFY_LIB_H

#include <string>
#include <chrono>
#include <optional>
#include "json-fetcher.h"

class SpotifyClient
{
public:
    SpotifyClient(std::string refresh_token, std::string client_id, std::string client_secret);

    struct NowPlaying
    {
    public:
        std::string artist;
        std::string track_name;
        std::string album;
        bool is_playing;
    };

    std::optional<NowPlaying> getNowPlaying();

private:
    JSONFetcher *fetcher;

    std::string client_id;
    std::string client_secret;

    std::string refresh_token;
    std::string access_token;

    std::chrono::time_point<std::chrono::system_clock> access_token_expiry;

    void refreshAccessToken();
    JSONFetcher::APIResponse apiQuery(std::string endpoint);
};

#endif /*SPOTIFY_LIB_H*/