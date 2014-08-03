/*
 * Thread-safe way to get the date
 */

#ifndef H_DATE
#define H_DATE

#include <mutex>
#include <ctime>
#include <chrono>
#include <sstream>

class Date
{
    std::string format;
    std::string unknown;
    static std::mutex mutex;

public:
    Date(std::string format = "%Y/%m/%d", std::string unknown = "Unknown")
        : format(format), unknown(unknown)
    { }

    std::string getDate();
};

#endif
