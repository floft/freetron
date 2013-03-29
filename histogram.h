/*
 * A class that creates a histogram for a 2D array of grayscale pixel values.
 * This is used to determine the threshold value.
 */

#ifndef H_HISTOGRAM
#define H_HISTOGRAM

#include <vector>
#include <iomanip> // TODO: remove

#include "math.h"
#include "options.h"

class Histogram
{
    int total;
    std::vector<int> graph;
    const std::vector<std::vector<unsigned char>>& img;

public:
    // Each inner vector is assumed to have the same length
    Histogram(const std::vector<std::vector<unsigned char>>& img);

    // Auto threshold. Specify the initial threshold to use to determine the
    // foreground and background.
    unsigned char threshold(unsigned char initial) const;
};

#endif
