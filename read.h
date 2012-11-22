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
#include "image.h"
#include "data.h"
#include "math.h"

using namespace std;
using namespace Magick;

// See if v is in boxes
bool inVector(const vector< vector<Coord> >& boxes, vector<Coord> v);

// Determine ID number from boxes 2-11
unsigned int findID(Pixels& img, const vector< vector<Coord> >& boxes,
	const unsigned int& max_x, const unsigned int& max_y,
	Image& image);

// Find x value of filled circle aligned with a point (x,y)
vector<unsigned int> findFilled(Pixels& img,
	const unsigned int& x,      const unsigned int& y,
        const unsigned int& stop_x, const unsigned int& max_y,
	Image& image);

#endif
