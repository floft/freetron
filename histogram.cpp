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

// Algorithm from:
//   http://www.heppenstall.ca/academics/doc/472/CIS472.Seminar03.Slides.1.ppt
unsigned char Histogram::threshold(unsigned char initial) const
{
    typedef std::vector<int>::size_type size_type;
    typedef std::vector<int>::const_iterator iterator;

    /* 
     * This algorithm is from the slides. But until I fix the findBlack()
     * algorithm, I don't really know which works better.
     *
     * TODO: retest this after fixing findBlack
     *
    int iterations = 0;
    double prev;
    double avg = initial;

    do
    {
        prev = avg;

        long long int blackMean = 0;
        long long int blackTotal = 0;

        for (int i = 0; i < avg; ++i)
        {
            blackMean += i*graph[i];
            blackTotal += graph[i];
        }

        long long int whiteMean = 0;
        long long int whiteTotal = 0;

        for (size_type i = avg; i < graph.size(); ++i)
        {
            whiteMean += i*graph[i];
            whiteTotal += graph[i];
        }

        if (blackTotal > 0 && whiteTotal > 0)
            avg = 0.5*(blackMean/blackTotal + whiteMean/whiteTotal);
        else
            break;

        ++iterations;
    } while (std::abs(prev - avg) > HIST_DIFF && iterations < HIST_MAX);

    return avg;*/

    // This is my algorithm that currently works better than the one above.
    const iterator half = graph.begin() + initial;
    iterator black_max = std::max_element(graph.begin(), half);
    iterator white_max = std::max_element(half, graph.end());

    if (black_max != half && white_max != graph.end())
        return 0.5*((black_max-graph.begin()) + (white_max-graph.begin()));

    return initial;
}
