/*
 * Code to find a box from a pixel, determine if it's a box, etc.
 *
 * Useful links on edge detection:
 *  http://www.m-hikari.com/ams/ams-password-2008/ams-password29-32-2008/nadernejadAMS29-32-2008.pdf
 *
 * Simplification of array of points using the Ramer-Douglas-Peucker algorithm:
 *  http://karthaus.nl/rdp/js/rdp.js
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
#include "options.h"
#include "pixels.h"
#include "forget.h"
#include "maputils.h"

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

// Used to return both point to jump to (if we had to go back a ways
// at a dead end) and the next direction to go
struct EdgePair
{
    Coord point;
    int index;

    EdgePair(const Coord& p, int i)
        :point(p), index(i) { }
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

// Determine if it's a box and calculate midpoint, width, and height
class Box
{
    // Width/height of box
    int w = 0;
    int h = 0;

    // Aspect ratio of this "box"
    double ar = 0;

    // Whether or not we have discovered the corners
    bool possibly_valid = false;

    // The calculated points
    Coord mp, topleft, topright, bottomleft, bottomright;

    // TODO: const
    Pixels& img;
    const Blobs& blobs;

    // Label of this pixel
    int label = Blobs::default_label;

    // Store diagonal information
    BoxData& data;

    // Used to walk the edge of a box. Relative coordinates
    // around the current position.
    static const std::array<Coord, 8> matrix;

public:
    Box(Pixels& pixels, const Blobs& blobs, const Coord& point, BoxData& data);

    bool valid();
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

    // Find the next pixel on edge by finding index of matrix used to move, or
    // if no available locations, moving back through our history till we can move
    // and returning that point and the index of the matrix to use to move.
    EdgePair findEdge(const Coord& p, const std::vector<Coord>& path) const;
    
    // Find next pixel to go to when walking edge, returns index of matrix
    // or -1 if all are black
    int findIndex(const Coord& p, const std::vector<Coord>& path,
        bool check_path = true) const;
};

#endif
