#ifndef RADIO6_CLIENT_H
#define RADIO6_CLIENT_H

#include <string>
#include <chrono>
#include "json-fetcher.h"

class Radio6Client
{
public:
    Radio6Client();
    ~Radio6Client();
    struct NowPlaying
    {
    public:
        std::string artist;
        std::string track_name;
    };

    NowPlaying getNowPlaying();

private:
    JSONFetcher *fetcher;
};

#endif /*RADIO6_CLIENT_H*/