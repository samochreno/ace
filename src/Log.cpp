#include "Log.hpp"

#include <string>
#include <ctime>

namespace Ace::Log
{
    auto CreateTimestamp() -> std::string
    {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
        return buf;
    }
}
