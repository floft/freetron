/*
 * Code to read the values
 */

#ifndef H_READ
#define H_READ

#include <map>
#include <cmath>
#include <vector>
#include <string>
#include <Magick++.h>

#include "options.h"
#include "data.h"
#include "math.h"
#include "box.h"

using namespace std;
using namespace Magick;

// See if the boxes are vertical
bool vertical(const vector<Coord>& boxes,
	const unsigned int& start_box, const unsigned int& end_box);

// Determine ID number from boxes 2-11
unsigned int findID(Pixels& img, const vector<Coord>& boxes,
	const unsigned int& max_x, const unsigned int& max_y,
	const unsigned int& box_width, Image& image);

// Determine answer black from max colors
double answerBlack(Pixels& img, const vector<Coord>& boxes,
	const unsigned int& start_box, const unsigned int& end_box,
	const unsigned int& start_x, const unsigned int& stop_x,
	const unsigned int& max_y,
	const unsigned int& box_width, const unsigned int& bubble_jump);

// Find which bubbles are filled, return x value
vector<unsigned int> findFilled(Pixels& img,
	const unsigned int& x, const unsigned int& y,
        const unsigned int& stop_x, const unsigned int& max_y,
	const unsigned int& box_width, const unsigned int& bubble_jump,
	const double& answer_black, Image& image);

#endif
