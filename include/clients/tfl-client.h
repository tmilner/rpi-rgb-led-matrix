#ifndef TFL_CLIENT_H
#define TFL_CLIENT_H

#include "clients/json-fetcher.h"
#include <string>
#include <vector>

class TflClient {
public:
  TflClient();
  ~TflClient();
  TflClient(const TflClient&) = delete;
  TflClient& operator=(const TflClient&) = delete;
  struct Arrival {
  public:
    std::string busName;
    int secondsUntilArrival;
    std::string getDisplayString();
    Arrival(std::string busName, int secondsUntilArrival) {
      this->busName = busName;
      this->secondsUntilArrival = secondsUntilArrival;
    }
  };

  std::vector<Arrival> getBusArrivals(std::string stopCode);

private:
  JSONFetcher *fetcher;
};

#endif /*TFL_CLIENT_H*/
