/*
 * Code to read the values
 */

#ifndef H_READ
#define H_READ

#include <map>
#include <cmath>
#include <vector>
#include <string>

#include "options.h"
#include "pixels.h"
#include "data.h"
#include "math.h"
#include "box.h"

// See if the boxes are vertical
bool vertical(const std::vector<Coord>& boxes,
	const int start_box, const int end_box);

// Determine ID number from boxes 2-11
int findID(Pixels& img, const std::vector<Coord>& boxes, BoxData* data);

// Determine answer black from max colors
double answerBlack(Pixels& img, const std::vector<Coord>& boxes,
	const int start_box, const int end_box,
	const int start_x, const int stop_x,
	const int box_width, const int bubble_jump);

// Find which bubbles are filled, return x value
std::vector<int> findFilled(Pixels& img,
	const int x, const int y,
        const int stop_x,
	const int box_width, const int bubble_jump,
	const double answer_black);

#endif
