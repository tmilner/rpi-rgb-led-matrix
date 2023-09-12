#include "json-fetcher.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <sstream>
#include <iostream>
using namespace std;

size_t write_data(
    const char *in,
    std::size_t size,
    std::size_t num,
    std::string *out)
{
    const std::size_t totalBytes(size * num);
    out->append(in, totalBytes);
    return totalBytes;
}

JSONFetcher::JSONFetcher()
{
    curl = curl_easy_init();
}

JSONFetcher::~JSONFetcher()
{
    curl_easy_cleanup(curl);
}

Json::Value JSONFetcher::fetch(std::string &url)
{
    CURL *curl = curl_easy_init();

    // Set remote URL.
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // Don't bother trying IPv6, which would increase DNS resolution time.
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    // Don't wait forever, time out after 10 seconds.
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); // Prevent "longjmp causes uninitialized stack frame" bug
    // Follow HTTP redirects if necessary.
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Response information.
    int httpCode(0);
    std::unique_ptr<std::string> httpData(new std::string());

    // Hook up data handling function.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    // Hook up data container (will be passed as the last parameter to the
    // callback handling function).  Can be any pointer type, since it will
    // internally be passed as a void pointer.
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

    // Run our HTTP GET command, capture the HTTP response code, and clean up.
    CURLcode code = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    if (code != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(code));
        throw std::runtime_error("Got bad code from CURL");
    }

    if (httpCode == 200)
    {
        std::cout << "\nGot successful response from " << url << std::endl;

        // Response looks good - done using Curl now.  Try to parse the results
        // and print them out.
        Json::Value jsonData;
        Json::Reader jsonReader;

        if (jsonReader.parse(*httpData, jsonData))
        {
            httpData.release();
            return jsonData;
        }
        else
        {
            std::cout << "Could not parse HTTP data as JSON" << std::endl;
            std::cout << "HTTP data was:\n"
                      << *httpData.get() << std::endl;
            httpData.release();

            throw std::runtime_error("Failed to Parse JSON");
        }
    }
    else
    {
        httpData.release();
        std::cout << "Non 200 from " << url << ". Code = " << httpCode << " Curl code " << curl_easy_strerror(code) << std::endl;
        throw std::runtime_error("Got non 200 Response from URL");
    }
}