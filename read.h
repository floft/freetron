/*
 * Code to read the values
 */

#ifndef H_READ
#define H_READ

#include <map>
#include <cmath>
#include <vector>
#include <string>

#include "box.h"
#include "math.h"
#include "data.h"
#include "blobs.h"
#include "pixels.h"
#include "outline.h"
#include "options.h"

// Set this to something that can't be detected on the form
const int DefaultID = -1;

// Data to keep about a bubble
struct Bubble 
{
    int radius;
    int label;
    Coord coord;

    Bubble(int r, int l, Coord c)
        : radius(r), label(l), coord(c) { }
};

// See if the boxes are vertical
bool vertical(const std::vector<Coord>& boxes,
    const int start_box, const int end_box);

// Determine ID number from boxes 2-11
int findID(Pixels& img, const Blobs& blobs,
    const std::vector<Coord>& boxes, const Data& data);

// Find which of the answers is filled
std::vector<Answer> findAnswers(Pixels& img, const Blobs& blobs,
    const std::vector<Coord>& boxes, const Data& data);

// Percentage of pixels that are marked with a certain label in a circle
// centered at c of radius r. Range: 0 to 1
double percentageLabel(const Pixels& img, const Blobs& blobs, const Bubble& b);

// Find all bubbles within the rectangle from p1 to p2
std::vector<Bubble> findBubbles(Pixels& img, const Blobs& blobs, const int diag,
    const Coord& a, const Coord& b);

#endif
