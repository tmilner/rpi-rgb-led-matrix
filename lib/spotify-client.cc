#include "spotify-client.h"

#include <iostream>

SpotifyClient::SpotifyClient(std::string refresh_token, std::string client_id, std::string client_secret)
{
    this->fetcher = new JSONFetcher();

    this->client_id = client_id;
    this->client_secret = client_secret;

    this->refresh_token = refresh_token;
    this->access_token = "";
    this->access_token_expiry = std::chrono::system_clock::now();
}

std::optional<SpotifyClient::NowPlaying> SpotifyClient::getNowPlaying()
{
    JSONFetcher::APIResponse response = this->apiQuery("v1/me/player?market=GB");
    if (response.code == 200 && response.body.has_value())
    {
        SpotifyClient::NowPlaying nowPlaying;
        auto item = response.body.value()["item"];

        nowPlaying.album = item["album"]["name"].asString();
        nowPlaying.artist = item["artists"][0]["name"].asString();
        nowPlaying.track_name = item["name"].asString();
        nowPlaying.is_playing = response.body.value()["is_playing"].asBool();

        return std::optional<SpotifyClient::NowPlaying>(nowPlaying);
    }
    if (response.code == 204)
    {
        return std::nullopt;
    }
    else
    {
        throw std::runtime_error("Unexpected response code when fetching now playing data");
    }
}

JSONFetcher::APIResponse SpotifyClient::apiQuery(std::string endpoint)
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

void SpotifyClient::refreshAccessToken()
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