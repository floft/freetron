#include "log.h"

// std::localtime not thread safe
static std::mutex localtime_lock;

void log(const std::string& msg, const LogType prefix, const bool err)
{
    if (DEBUG && err)
    {
        std::cerr << prefix << ": " << msg << std::endl;
    }

    if (LOGGING)
    {
        // std::localtime isn't thread safe
        std::unique_lock<std::mutex> lck(localtime_lock);
        
        char timestamp[50] = "";
        time_t now = std::time(nullptr);
        std::strftime(timestamp, 50, "%c %Z", std::localtime(&now));

        std::ofstream log(LOG_FILE, std::ios_base::out|std::ios_base::app);
        log << timestamp << "\t" << prefix << ": " << msg << std::endl;
    }
}

std::ostream& operator<<(std::ostream& os, const LogType& t)
{
    switch(t)
    {
        case LogType::Error:   return os << "Error";
        case LogType::Warning: return os << "Warning";
        case LogType::Notice:  return os << "Notice";
        default: return os << "Unknown";
    }
}
