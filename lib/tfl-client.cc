#include "tfl-client.h"

#include <iostream>
#include <algorithm>
#include <string>

TflClient::TflClient()
{
    this->fetcher = new JSONFetcher();
}

TflClient::~TflClient()
{
}

bool compareArrivalTimes(const TflClient::Arrival &a, const TflClient::Arrival &b)
{
    return a.secondsUntilArrival < b.secondsUntilArrival;
}

std::string TflClient::Arrival::getDisplayString()
{
    if (this->secondsUntilArrival < 60)
    {
        return this->busName.append(": ").append(std::to_string(this->secondsUntilArrival)).append("seconds");
    }
    else
    {
        int arrivalInMins = this->secondsUntilArrival / 60;
        return this->busName.append(": ").append(std::to_string(arrivalInMins)).append("mins");
    }
}

std::vector<TflClient::Arrival> TflClient::getButArrivals(std::string busCode)
{
    JSONFetcher::APIResponse response = this->fetcher->fetch("GET", NULL, "https://api.tfl.gov.uk/StopPoint/" + busCode + "/Arrivals", "");
    if (response.code == 200 && response.body.has_value())
    {
        std::vector<TflClient::Arrival> arrivals = std::vector<TflClient::Arrival>();
        Json::Value jsonData = response.body.value();

        for (int i = 0; i < jsonData.size(); i++)
        {
            Json::Value bus = jsonData[i];
            int timeToStationSeconds = bus["timeToStation"].asInt();

            TflClient::Arrival arrival(bus["lineName"].asString(), bus["timeToStation"].asInt());
            arrivals.push_back(arrival);
        }

        std::sort(arrivals.begin(), arrivals.end(), compareArrivalTimes);
        return arrivals;
    }
    else
    {
        throw std::runtime_error("Unexpected response code when fetching now playing data");
    }
}

JSONFetcher::APIResponse TflClient::apiQuery(std::string endpoint)
{
    auto now = std::chrono::system_clock::now();

    // check for expiry... do it 10s before it expires if we can.
    if (std::chrono::duration_cast<std::chrono::seconds>(now - this->access_token_expiry).count() >= -10)
        this->refreshAccessToken();

    std::string auth_header = "Authorization: Bearer " + this->access_token;
    std::string accept = "Accept: application/json";

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header.c_str());
    headers = curl_slist_append(headers, accept.c_str());

    std::string url = "https://api.spotify.com/" + endpoint;
    std::cout << "API Query - url = " << url << ", auth_header " << auth_header << std::endl;

    return this->fetcher->fetch("GET", headers, url, "");
}

void TflClient::refreshAccessToken()
{
    std::string postData = "grant_type=refresh_token&refresh_token=" + this->refresh_token + "&client_id=" + this->client_id + "&client_secret=" + this->client_secret;
    std::string content_type_header = "Content-Type: application/x-www-form-urlencoded";
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, content_type_header.c_str());

    std::cout << "Refresh Access token - " << this->refresh_token << ", " << this->client_id << ", " << this->client_secret << ", body = " << postData << ", header " << content_type_header << std::endl;
    JSONFetcher::APIResponse token = this->fetcher->fetch("POST", headers, "https://accounts.spotify.com/api/token", postData);
    if (token.code == 200 && token.body.has_value())
    {
        this->access_token = token.body.value()["access_token"].asString();
        auto expiry = std::chrono::system_clock::now();
        expiry += std::chrono::seconds(token.body.value()["expires_in"].asInt());
        this->access_token_expiry = expiry;
    }
    else
    {
        std::cerr << "FAILED TO REFRESH TOKEN" << std::endl;
        throw std::runtime_error("Got non 200 Response when refreshing access token");
    }
};