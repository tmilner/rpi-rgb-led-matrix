#include "tfl-client.h"

#include <algorithm>
#include <iostream>
#include <string>

TflClient::TflClient() { this->fetcher = new JSONFetcher(); }

TflClient::~TflClient() {}

bool compareArrivalTimes(const TflClient::Arrival &a,
                         const TflClient::Arrival &b) {
  return a.secondsUntilArrival < b.secondsUntilArrival;
}

std::string TflClient::Arrival::getDisplayString() {
  if (this->secondsUntilArrival < 60) {
    return this->busName.append(": ")
        .append(std::to_string(this->secondsUntilArrival))
        .append("s");
  } else if (this->secondsUntilArrival == 60) {
    return this->busName.append(":1m");
  } else {
    int arrivalInMins = this->secondsUntilArrival / 60;
    return this->busName.append(":")
        .append(std::to_string(arrivalInMins))
        .append("m");
  }
}

std::vector<TflClient::Arrival> TflClient::getButArrivals(std::string busCode) {
  JSONFetcher::APIResponse response = this->fetcher->fetch(
      "GET", NULL, "https://api.tfl.gov.uk/StopPoint/" + busCode + "/Arrivals",
      "");
  if (response.code == 200 && response.body.has_value()) {
    std::vector<TflClient::Arrival> arrivals =
        std::vector<TflClient::Arrival>();
    Json::Value jsonData = response.body.value();

    for (int i = 0; i < jsonData.size(); i++) {
      Json::Value bus = jsonData[i];
      int timeToStationSeconds = bus["timeToStation"].asInt();

      TflClient::Arrival arrival(bus["lineName"].asString(),
                                 bus["timeToStation"].asInt());
      arrivals.push_back(arrival);
    }

    std::sort(arrivals.begin(), arrivals.end(), compareArrivalTimes);
    return arrivals;
  } else {
    throw std::runtime_error(
        "Unexpected response code when fetching now playing data");
  }
}
