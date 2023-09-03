#include <curl/curl.h>
#include <string.h>
#include <string>

using namespace rgb_matrix;

class Radio6NowPlaying { 
        CURL *curl;
    public:
        void update();
        std::string getContents();
        void getIcon() ;
};
