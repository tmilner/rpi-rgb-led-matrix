#include "radio6-client.h"

#include <iostream>

Radio6Client::Radio6Client()
{
    this->fetcher = new JSONFetcher();
}

Radio6Client::~Radio6Client()
{
}

Radio6Client::NowPlaying Radio6Client::getNowPlaying()
{
    JSONFetcher::APIResponse response = this->fetcher->fetch("GET", NULL, "https://nowplaying.jameswragg.com/api/bbc6music?limit=1", "");
    
    if (response.code == 200 && response.body.has_value())
    {
        Radio6Client::NowPlaying nowPlaying;
        auto item = response.body.value()["tracks"][0];

        nowPlaying.artist = item["artist"].asString();
        nowPlaying.track_name = item["name"].asString();

        return nowPlaying;
    }
    else
    {
        throw std::runtime_error("Unexpected response code when fetching now playing data");
    }
}
