/*
 * Code to determine the rotation of the image
 */

#ifndef H_ROTATE
#define H_ROTATE

#include <cmath>
#include <iostream> //TODO:REMOVE THIS
#include <vector>
#include <algorithm>
#include <Magick++.h>

#include "options.h"
#include "math.h"
#include "image.h"

using namespace std;
using namespace Magick;

// Find top left coordinate of the box containing point (x,y)
Coordinate findTopLeft(Pixels& img,
	const unsigned int& x,     const unsigned int& y,
	const unsigned int& max_x, const unsigned int& max_y);

// Find top right coordinate of the box containing point (x,y)
Coordinate findTopRight(Pixels& img,
	const unsigned int& x,     const unsigned int& y,
	const unsigned int& max_x, const unsigned int& max_y);

// Find boxes on left of image
vector< vector<unsigned int> > findBoxes(Pixels& img,
	const unsigned int& max_x, const unsigned int& max_y,
	unsigned int& box_width, Image& image);

// Find boxes after rotating, verify they are similar distance from edge
vector< vector<unsigned int> > findRealBoxes(Pixels& img,
	const unsigned int& max_x, const unsigned int& max_y,
	unsigned int& box_width, Image& image);

// Determine average slope of 2 boxes if they are close enough, otherwise
// continue till finding closer boxes
//double findRotation(Pixels& img, const vector< vector<unsigned int> >& boxes,
//	unsigned int& ret_x, unsigned int& ret_y,
//	const unsigned int& max_x, const unsigned int& max_y, Image& image);
double findRotation(Pixels& img, unsigned int& ret_x, unsigned int& ret_y,
	const unsigned int& max_x, const unsigned int& max_y, Image& image);

#endif
