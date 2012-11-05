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

using namespace std;
using namespace Magick;

// Average color of all pixels within radius r of (x,y)
// 0 = complete white, 1 = complete black
double averageColor(Pixels& img,   const unsigned int& x,
	const unsigned int& y,     const unsigned int& r,
	const unsigned int& max_x, const unsigned int& max_y);

#endif
