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

// Find top-left and bottom-left boxes and give rotation to make them vertical
double findRotation(Pixels& img, unsigned int& ret_x, unsigned int& ret_y,
	const unsigned int& max_x, const unsigned int& max_y);

#endif
