#include "date.h"

typedef std::chrono::time_point<std::chrono::system_clock> time_point;

// This is static so we only ever access std::localtime from
// one thread, which is the entire purpose of this class
std::mutex Date::mutex;

std::string Date::getDate()
{
    std::unique_lock<std::mutex> lock(mutex);

    // Get the current date
    time_point now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* date_time = std::localtime(&in_time_t);

    // Since the mutex is static, this can be static as well
    static const size_t size = 15;
    static char buf[size];

    if (std::strftime(buf, size, format.c_str(), date_time) == 0)
        return unknown;

    return std::string(buf);
}
