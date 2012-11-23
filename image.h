/*
 * Code to determine the rotation of the image
 */

#ifndef H_IMAGE
#define H_IMAGE

#include <cmath>
#include <vector>
#include <algorithm>
#include <Magick++.h>

#include "data.h"
#include "math.h"
#include "options.h"

using namespace std;
using namespace Magick;

// Return if the pixel at (x,y) is black
bool isBlack(Pixels& img, const unsigned int& x, const unsigned int& y);

// Find the midpoint between two points
Coord midPoint(const Coord& p1, const Coord& p2);

// Go a direction until MAX_ERROR white pixels, return last black point
unsigned int goUp(Pixels& img, const Coord& p, const Coord& orig);
unsigned int goLeft(Pixels& img, const Coord& p, const Coord& orig);
unsigned int goDown(Pixels& img,
	const Coord& p, const Coord& orig,
	const unsigned int& max_y);
unsigned int goRight(Pixels& img,
	const Coord& p, const Coord& orig,
	const unsigned int& max_x);

// Find extreme points in a direction
Coord leftmost(Pixels& img, const Coord& original,
	const unsigned int& max_x, const unsigned int& max_y);
Coord topmost(Pixels& img, const Coord& original,
	const unsigned int& max_x, const unsigned int& max_y);
Coord rightmost(Pixels& img, const Coord& original,
	const unsigned int& max_x, const unsigned int& max_y);
Coord bottommost(Pixels& img, const Coord& original,
	const unsigned int& max_x, const unsigned int& max_y);

// Average color of all pixels within radius r of (x,y)
// 0 = complete white, 1 = complete black
double averageColor(Pixels& img,   const unsigned int& x,
	const unsigned int& y,     const unsigned int& r,
	const unsigned int& max_x, const unsigned int& max_y);

#endif
