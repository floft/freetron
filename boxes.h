/*
 * Code to determine find the boxes
 */

#ifndef H_BOXES
#define H_BOXES

#include <cmath>
#include <vector>
#include <iostream>	//TODO: remove this
#include <algorithm>
#include <Magick++.h>

#include "options.h"
#include "math.h"
#include "image.h"

using namespace std;
using namespace Magick;

// Find boxes in the image returns { { x1, y1, x2, y2 }, ... }
// where (x1,y1) is the top-left point and (x2,y2) is the bottom-
// right point.
vector< vector<Coord> > findBoxes(Pixels& img,
	const unsigned int& max_x, const unsigned int& max_y,
	Image& image);

#endif
