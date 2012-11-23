/*
 * Code to determine find the boxes
 */

#ifndef H_BOXES
#define H_BOXES

#include <cmath>
#include <vector>
#include <algorithm>
#include <Magick++.h>

#include "options.h"
#include "math.h"
#include "image.h"

using namespace std;
using namespace Magick;

// Functions needed due to using nested vectors
bool box_sort(const Coord& v1, const Coord& v2);

// Find boxes in the image returns { { midpoint_x, midpoint_y }, ... }
vector<Coord> findBoxes(Pixels& img,
	const unsigned int& max_x, const unsigned int& max_y);

#endif
