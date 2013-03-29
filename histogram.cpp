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
}

// Search for max black value and max white value. Use the shade halfway
// in between.
unsigned char Histogram::threshold(unsigned char initial) const
{
    // TODO: fix this algorithm
    return initial;

    typedef std::vector<int>::const_iterator iterator;

    const iterator half = graph.begin() + initial;
    iterator black_max = std::max_element(graph.begin(), half);
    iterator white_max = std::max_element(half, graph.end());

    if (black_max != half && white_max != graph.end())
        return 0.5*((black_max-graph.begin()) + (white_max-graph.begin()));

    return initial;
}
