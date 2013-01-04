/*
 * Code to determine find the boxes
 */

#ifndef H_BOXES
#define H_BOXES

#include <cmath>
#include <vector>
#include <algorithm>

#include "options.h"
#include "pixels.h"
#include "blobs.h"
#include "data.h"
#include "math.h"
#include "box.h"

// Find boxes in the image returns { Coord(midpoint_x, midpoint_y), ... }
std::vector<Coord> findBoxes(Pixels& img, BoxData& data);

#endif
