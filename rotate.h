/*
 * Code to determine the rotation of the image
 */

#ifndef H_ROTATE
#define H_ROTATE

#include <vector>

#include "data.h"
#include "pixels.h"

// Find top-left and bottom-left boxes and give rotation to make them vertical
double findRotation(const Pixels& img, const std::vector<Coord>& boxes, Coord& ret_coord);

#endif
