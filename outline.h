/*
 * Find the points around a specific object
 *
 * Useful links on edge detection:
 *  http://www.m-hikari.com/ams/ams-password-2008/ams-password29-32-2008/nadernejadAMS29-32-2008.pdf
 */

#ifndef H_OUTLINE
#define H_OUTLINE

#include <array>
#include <vector>
#include <cstdbool>
#include <algorithm>

#include "log.h"
#include "data.h"
#include "blobs.h"
#include "options.h"
#include "pixels.h"

// Used to return both point to jump to (if we had to go back a ways
// at a dead end) and the next direction to go
struct EdgePair
{
    Coord point;
    int index;

    EdgePair(const Coord& p, int i)
        :point(p), index(i) { }
};

// Get the outline of the object
class Outline
{
    const Pixels& img;
    const Blobs& blobs;

    // Label of this pixel
    int label = Blobs::default_label;

    // Save the outline of this object
    std::vector<Coord> path;

    // Did we find the outline?
    bool found = false;

    // Used to walk the edge of a box. Relative coordinates
    // around the current position.
    static const std::array<Coord, 8> matrix;

public:
    Outline(const Pixels& pixels, const Blobs& blobs, const Coord& point,
        const int max_length);

    bool good() const { return found; }

    const std::vector<Coord>& points() const { return path; }

private:
    // Find the next pixel on edge by finding index of matrix used to move, or
    // if no available locations, moving back through our history till we can move
    // and returning that point and the index of the matrix to use to move.
    EdgePair findEdge(const Coord& p) const;
    
    // Find next pixel to go to when walking edge, returns index of matrix
    // or -1 if all are black
    int findIndex(const Coord& p) const;
};

#endif
