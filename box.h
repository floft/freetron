/*
 * Code to find a box from a pixel, determine if it's a box, etc.
 */

#ifndef H_BOX
#define H_BOX

#include "data.h"
#include "blobs.h"
#include "pixels.h"

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

// Determine if it's a box and properties
class Box
{
    int w = 0;
    int h = 0;
    int diag = 0;
    double ar = 0;
    bool valid_box = false;
    Coord mp, topleft, topright, bottomleft, bottomright;

    Pixels& img;
    const Blobs& blobs;
    int label = Blobs::default_label;

public:
    Box(Pixels& pixels, const Blobs& blobs, const Coord& point);

    inline bool valid() const { return valid_box; }
    inline int width() const  { return w; }
    inline int height() const { return h; }
    inline int diagonal() const { return diag; }
    inline double aspect() const  { return ar; }
    inline const Coord& midpoint() const { return mp; }

private:
    // Determine if average colors of pixels inside the box are greater
    // than MIN_BLACK and average color of pixels around the box are less
    // than MAX_BLACK.
    bool validBoxColor() const;
};

#endif
