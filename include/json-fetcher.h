#ifndef JSONFETCHER_HPP
#define JSONFETCHER_HPP

#include <string>
#include <json/json.h>
/**
 * A non-threadsafe simple libcURL-easy based HTTP downloader
 */
class JSONFetcher {
public:
    JSONFetcher();
    ~JSONFetcher();
    /**
     * Download a file using HTTP GET and store in in a std::string
     * @param url The URL to download
     * @return The download result
     */
    Json::Value fetch(std::string& url);
private:
    void* curl;
};

#endif  /* JSONFETCHER_HPP */
