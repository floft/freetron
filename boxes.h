/*
 * Code to determine find the boxes
 */

#ifndef H_BOXES
#define H_BOXES

#include <vector>

#include "data.h"
#include "blobs.h"
#include "pixels.h"

// Find boxes in the image returns { Coord(midpoint_x, midpoint_y), ... }
std::vector<Coord> findBoxes(Pixels& img, const Blobs& blobs, Data& data);

#endif
