/*
 * Code to determine the rotation of the image
 */

#ifndef H_ROTATE
#define H_ROTATE

#include <cmath>
#include <vector>
#include <algorithm>
#include <Magick++.h>

#include "options.h"
#include "math.h"
#include "image.h"

using namespace std;
using namespace Magick;

// See if this pixel is in a box
BoxData analyzeBox(Pixels& img, const Coord& p,
	const unsigned int& max_x, const unsigned int& max_y);

// Find top-left and bottom-left boxes and give rotation to make them vertical
double findRotation(Pixels& img, Coord& ret_coord,
	const unsigned int& max_x, const unsigned int& max_y);

#endif
