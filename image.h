/*
 * Code to determine the rotation of the image
 */

#ifndef H_IMAGE
#define H_IMAGE

#include <cmath>
#include <vector>
#include <algorithm>
#include <Magick++.h>

#include "math.h"
#include "options.h"

using namespace std;
using namespace Magick;

// Average color of all pixels within radius r of (x,y)
// 0 = complete white, 1 = complete black
double averageColor(Pixels& img,   const unsigned int& x,
	const unsigned int& y,     const unsigned int& r,
	const unsigned int& max_x, const unsigned int& max_y);

// Return if the pixel at (x,y) is black
bool isBlack(Pixels& img, const unsigned int& x, const unsigned int& y);

// Find the midpoint between two points
Coordinate midPoint(const Coordinate& p1, const Coordinate&p2);

// Find the leftmost point defaulting to the top if there are multiple
Coordinate leftmost(Pixels& img,
	const unsigned int& x, const unsigned int& y,
	const unsigned int& max_x, const unsigned int& max_y);

// Find the rightmost point defaulting to the bottom if there are multiple
Coordinate rightmost(Pixels& img,
	const unsigned int& x, const unsigned int& y,
	const unsigned int& max_x, const unsigned int& max_y);

#endif
