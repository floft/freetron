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

using namespace std;

// Find top-left and bottom-left boxes and give rotation to make them vertical
double findRotation(Pixels& img, Coord& ret_coord,
	const unsigned int max_x, const unsigned int max_y, BoxData* box_data);

#endif
