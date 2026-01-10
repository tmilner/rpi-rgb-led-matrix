#ifndef JSONFETCHER_HPP
#define JSONFETCHER_HPP

#include <string>
#include <optional>
#include <json/json.h>
#include <curl/curl.h>

/**
 * A non-threadsafe simple libcURL-easy based HTTP downloader
 */
class JSONFetcher
{
public:
    JSONFetcher();
    ~JSONFetcher();

    struct APIResponse
    {
        int code;
        std::optional<Json::Value> body;
        APIResponse(int code, std::optional<Json::Value> body): code(code), body(body){};
    };
    /**
     * Download a file using HTTP GET and store in in a std::string
     * @param url The URL to download
     * @return The download result
     */
    Json::Value fetch(std::string url);

    APIResponse fetch(std::string request, curl_slist *headers, std::string url, std::string body);

private:
    CURL *curl;
};

#endif /* JSONFETCHER_HPP */
