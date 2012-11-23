/*
 * Code to find a box from a pixel, determine if it's a box, etc.
 */

#ifndef H_BOX
#define H_BOX

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

// Average color of all pixels within radius r of (x,y)
// 0 = complete white, 1 = complete black
double averageColor(Pixels& img,
	const unsigned int& x, const unsigned int& y,
	const unsigned int& r,
	const unsigned int& max_x, const unsigned int& max_y);

// Determine if it's a box and calculate midpoint, width, and height
class Box
{
	// Dimensions of image
	unsigned int max_x;
	unsigned int max_y;
	// Width/height of box
	unsigned int w;
	unsigned int h;
	// Aspect ratio of this "box"
	double ar;
	// The image, but don't ever delete this...
	Pixels* img;
	// The calculated midpoint
	Coord mp;
	// The diagonal based on the first few valid boxes
	static unsigned int diag;
	// Used to see if there's several of the same-sized boxes
	static vector<unsigned int> diags;

public:
	Box(); // Only used as a placeholder, then copy another box to it
	Box(Pixels& pixels, const Coord& point,
		const unsigned int& maxX, const unsigned int& maxY);

	bool valid();
	const unsigned int& width() const  { return w; }
	const unsigned int& height() const { return h; }
	const double& aspect() const  { return ar; }
	const Coord& midpoint() const { return mp; }

private:
	// See if diags contains any beyond error margins
	bool absurdDiagonal() const;
	
	// Find the midpoint between two points
	Coord midPoint(const Coord& p1, const Coord& p2) const;

	// Go a direction until MAX_ERROR white pixels, return last black point
	unsigned int goUp(const Coord& p, const Coord& orig) const;
	unsigned int goLeft(const Coord& p, const Coord& orig) const;
	unsigned int goDown(const Coord& p, const Coord& orig) const;
	unsigned int goRight(const Coord& p, const Coord& orig) const;

	// Find extreme points in a direction
	Coord leftmost(const Coord& point) const;
	Coord topmost(const Coord& point) const;
	Coord rightmost(const Coord& point) const;
	Coord bottommost(const Coord& point) const;
};

#endif
