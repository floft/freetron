/*
 * Code to find a box from a pixel, determine if it's a box, etc.
 */

#ifndef H_BOX
#define H_BOX

#include <map>
#include <cmath>
#include <array>
#include <vector>
#include <cstdbool>
#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "log.h"
#include "data.h"
#include "blobs.h"
#include "math.h"
#include "outline.h"
#include "options.h"
#include "pixels.h"

// Store data for each image separately (needed instead of some sort of static
// hack because we're using multithreading)
struct BoxData
{
    // Approximate width of the box
    int width = 0;
    // The diagonal based on the first few valid boxes
    int diag = 0;
    // Used to see if there's several of the same-sized boxes
    std::vector<int> diags;
};

// Find square around coordinates keeping it within the image bounds
class Square
{
    Coord topleft;
    Coord bottomright;
    Coord midpoint;

public:
    Square(const Pixels& img, const int x, const int y, const int r);
    inline const Coord& topLeft()     const { return topleft; }
    inline const Coord& bottomRight() const { return bottomright; }
    inline const Coord& midPoint()    const { return midpoint; }
    bool in(const Coord& c) const;
};

// Average color of all pixels within radius r of (x,y)
// 0 = complete white, 1 = complete black
double averageColor(const Pixels& img,
    const int x, const int y,
    const int r);

// Determine if it's a box and properties
class Box
{
    // Width/height of box
    int w = 0;
    int h = 0;

    // Aspect ratio of this "box"
    double ar = 0;

    // The calculated points
    Coord mp, topleft, topright, bottomleft, bottomright;

    // Whether or not we have discovered the corners
    bool valid_box = false;

    Pixels& img;
    const Blobs& blobs;

    // Label of this pixel
    int label = Blobs::default_label;

    // Store diagonal information
    BoxData& data;

public:
    Box(Pixels& pixels, const Blobs& blobs, const Coord& point, BoxData& data);

    inline bool valid() const { return valid_box; }
    inline int width() const  { return w; }
    inline int height() const { return h; }
    inline double aspect() const  { return ar; }
    inline const Coord& midpoint() const { return mp; }

private:
    // Find the midpoint between two points
    Coord midPoint(const Coord& p1, const Coord& p2) const;

    // Determine if average colors of pixels inside the box are greater
    // than MIN_BLACK and average color of pixels around the box are less
    // than MAX_BLACK.
    bool validBoxColor() const;

    // To determine the corners, we'll look for the farthest point from the initial
    // point in a box and the farthest point from that. Those will be a diagonal.
    // Next, look for the two points farthest from that line for the other two
    // corners (making the assumption that this is a quadrilateral).
    Coord farthestFromPoint(const Coord& p,
        const std::vector<Coord>& points) const;
    Coord farthestFromLine(const Coord& p1, const Coord& p2,
        const std::vector<Coord>& points) const;
};

#endif
