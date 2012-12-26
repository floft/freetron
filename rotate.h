/*
 * Code to determine the rotation of the image
 */

#ifndef H_ROTATE
#define H_ROTATE

#include <cmath>
#include <vector>
#include <algorithm>

#include "options.h"
#include "pixels.h"
#include "math.h"
#include "box.h"

// Find top-left and bottom-left boxes and give rotation to make them vertical TODO: add const
double findRotation(Pixels& img, Coord& ret_coord, BoxData* data);

#endif
