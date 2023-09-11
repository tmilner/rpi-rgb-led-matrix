#ifndef LINEUPDATED_HPP
#define LINEUPDATED_HPP

#include <string>
#include "json-fetcher.h"
#include <Magick++.h>

class LineUpdater
{
public:
    void updateLine();
    Magick::Image *getIcon();
    std::string *getLine()
    {
        return &this->line;
    }

protected:
    std::string line;
};

#endif /*LINEUPDATED_HPP*/