/*
 * Code to find a box from a pixel, determine if it's a box, etc.
 */

#ifndef H_BOX
#define H_BOX

#include <cmath>
#include <vector>
#include <algorithm>

#include "data.h"
#include "math.h"
#include "options.h"
#include "pixels.h"

// Store data for each image separately (for multithreading)
struct BoxData
{
	// Approximate width of the box
	int width = 0;
	// The diagonal based on the first few valid boxes
	int diag = 0;
	// Used to see if there's several of the same-sized boxes
	std::vector<int> diags;
};

// Find square around coordinants keeping it within the image bounds
class Square
{
	Coord topleft;
	Coord bottomright;
	Coord midpoint;

public:
	Square(const Pixels& img, const int x, const int y, const int r);
	inline const Coord& topLeft()     const { return topleft; }
	inline const Coord& bottomRight() const { return bottomright; }
	inline const Coord& midPoint()    const { return midpoint; }
};

// Average color of all pixels within radius r of (x,y)
// 0 = complete white, 1 = complete black
double averageColor(const Pixels& img,
	const int x, const int y,
	const int r);

// Determine if it's a box and calculate midpoint, width, and height
class Box
{
	// Dimensions of image
	int max_x = 0;
	int max_y = 0;
	// Width/height of box
	int w = 0;
	int h = 0;
	// Aspect ratio of this "box"
	double ar = 0;
	// The image, but don't ever delete this...
	const Pixels* img = nullptr;
	// The calculated midpoint
	Coord mp;
	// Store diagonal information
	BoxData* data;

	// TODO: remove this
	std::vector<Coord> coords;

public:
	Box() { } // Only used as a placeholder, then copy another box to it
	Box(const Pixels* pixels, const Coord& point, BoxData* data);

	bool valid();
	inline int width() const  { return w; }
	inline int height() const { return h; }
	inline double aspect() const  { return ar; }
	inline const Coord& midpoint() const { return mp; }

private:
	// See if diags contains any beyond error margins
	bool absurdDiagonal() const;
	
	// Find the midpoint between two points
	Coord midPoint(const Coord& p1, const Coord& p2) const;

	// Find most dense region of black around a point, i.e. get into the box
	Coord findDark(const Coord& p) const;

	// Go a direction until MAX_ERROR white pixels, return last black point
	int goUp(const Coord& p, const Coord& orig) const;
	int goLeft(const Coord& p, const Coord& orig) const;
	int goDown(const Coord& p, const Coord& orig) const;
	int goRight(const Coord& p, const Coord& orig) const;

	// Find extreme points in a direction
	Coord leftmost(const Coord& point) const;
	Coord topmost(const Coord& point) const;
	Coord rightmost(const Coord& point) const;
	Coord bottommost(const Coord& point) const;
};

#endif
