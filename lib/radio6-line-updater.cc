#include "radio6-line-updater.h"
#include <iostream>

Radio6LineUpdater::Radio6LineUpdater(JSONFetcher *fetcher,
                                     std::map<std::string, Magick::Image> *image_map)
{
    this->fetcher = fetcher;
    this->line = "Loading";
    this->url = std::string("https://nowplaying.jameswragg.com/api/bbc6music?limit=1");
    this->image_map = image_map;
    this->image_key = "radio6icon";

    Magick::Image tmp = (*image_map)[this->image_key];
    tmp.resize(Magick::Geometry(11, 11));

    (*this->image_map)[this->image_key] = tmp; 
}

Magick::Image *Radio6LineUpdater::getIcon()
{
    return &((*this->image_map)[this->image_key]);
}

void Radio6LineUpdater::updateLine()
{
    std::cout << "Fetching radio6 data from " << this->url << std::endl;

    try
    {
        Json::Value jsonData = fetcher->fetch(this->url);

        const std::string artist(jsonData["tracks"][0]["artist"].asString());
        const std::string track_name(jsonData["tracks"][0]["name"].asString());

        std::cout << "\tArtist: " << artist << std::endl;
        std::cout << "\tTrack Name: " << track_name << std::endl;
        std::cout << std::endl;

        this->line.clear();
        this->line.append(artist).append(" - ").append(track_name);
    }
    catch (std::runtime_error &e)
    {
        printf("Failed to fetch weather\n");
    }
}
