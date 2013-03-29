#include "histogram.h"

Histogram::Histogram(const std::vector<std::vector<unsigned char>>& img)
    : img(img)
{
    typedef std::vector<int>::size_type size_type;

    // This is unsigned char, so there's 0-255
    graph = std::vector<int>(256, 0);

    int h = img.size();
    int w = (h>0)?img[0].size():0;
    total = w*h;

    // Generate the graph by counting how many pixels are each shade. This
    // is easy with discrete values, would be more interesting with doubles.
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            ++graph[img[y][x]];

    if (DEBUG)
    {
        for (size_type i = 0; i < graph.size(); ++i)
        {
            std::cout << std::setw(7) << graph[i] << " ";

            if (!((i+1)%16))
                std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

// Search for two peaks, use the gray shade halfway in between
unsigned char Histogram::balancedThreshold(unsigned char initial) const
{
    typedef std::vector<int>::size_type size_type;
    typedef std::vector<int>::const_iterator iterator;

    // Find median
    int pixels = 0;
    unsigned char median = 0;

    for (size_type i = 0; i < graph.size(); ++i)
    {
        if (total/2 < pixels+graph[i])
        {
            median = i;
            break;
        }
        else
        {
            pixels += graph[i];
        }
    }

    std::cout << "Median: " << (int)median << std::endl;


    // Find percentage at 174
    int left = 0;

    for (int i = 0; i <= 174; ++i)
        left += graph[i];

    std::cout << "174: " << 1.0*left/total*100 << "%" << std::endl;


    // Centroid
    int weighted_total = 0;

    for (size_type i = 0; i < graph.size(); ++i)
        weighted_total += i*graph[i];

    std::cout << "Centroid: " << 1.0*weighted_total/total << std::endl;

    std::cout << "STDev: " << stdDev(graph) << std::endl;


    // Look for 90% (or 10%)
    pixels = 0;
    int breakpoint = 0;

    for (size_type i = 0; i < graph.size(); ++i)
    {
        pixels += graph[i];

        if (1.0*pixels/total > 0.1)
            break;

        breakpoint = i;
    }

    std::cout << "BP: " << breakpoint << std::endl;

    // Average two max values
    int avg = initial;
    const iterator half = graph.begin() + initial;
    iterator black_max = std::max_element(graph.begin(), half);
    iterator white_max = std::max_element(half, graph.end());

    if (black_max != half && white_max != graph.end())
        avg = 0.5*((black_max-graph.begin()) + (white_max-graph.begin()));

    std::cout << "Avg: " << avg << std::endl;

    return initial;
}
