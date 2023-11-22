#ifndef TFL_CLIENT_H
#define TFL_CLIENT_H

#include <string>
#include <chrono>
#include <vector>
#include "json-fetcher.h"

class TflClient
{
public:
    TflClient();
    ~TflClient();
    struct Arrival
    {
    public:
        std::string busName;
        int secondsUntilArrival;
        std::string getDisplayString();
        Arrival(std::string busName, int secondsUntilArrival)
        {
            this->busName = busName;
            this->secondsUntilArrival = secondsUntilArrival;
        }
    };

    std::vector<Arrival> getButArrivals(std::string stopCode);

private:
    JSONFetcher *fetcher;
};

#endif /*TFL_CLIENT_H*/