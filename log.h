/*
 * This is a little wrapper for error messages so that they can be written to
 * the screen while debugging and sent to a log while in production.
 */

#ifndef H_LOG
#define H_LOG

#include <ctime>
#include <mutex>
#include <string>
#include <fstream>
#include <iostream>

#include "options.h"

enum class LogType
{
    Error, Warning, Notice
};

std::ostream& operator<<(std::ostream& os, const LogType& t);

// err will write to std::cerr when debugging is enabled
void log(const std::string& msg, const LogType prefix = LogType::Error,
    const bool err = true);

#endif
