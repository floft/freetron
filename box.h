/*
 * Code to find a box from a pixel, determine if it's a box, etc.
 *
 * Useful links on edge detection:
 *  http://www.m-hikari.com/ams/ams-password-2008/ams-password29-32-2008/nadernejadAMS29-32-2008.pdf
 */

#ifndef H_BOX
#define H_BOX

#include <map>
#include <cmath>
#include <array>
#include <vector>
#include <stdexcept>
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

// Direction for walking edge (top left, top right...)
enum class Direction
{
	TL, TR, BL, BR, Unknown
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
	bool in(const Coord& c) const;
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
	// Whether or not we have discovered the corners
	bool possibly_valid = false;
	// The image	TODO: add const
	Pixels* img = nullptr;
	// The calculated points
	Coord mp, topleft, topright, bottomleft, bottomright;
	// Store diagonal information
	BoxData* data;

public:
	Box() { } // Only used as a placeholder, then copy another box to it
	Box(Pixels* pixels, const Coord& point, BoxData* data);

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

	// Determine average color of all pixels within corners of box
	double boxColor() const;

	// Determine probable direction based on 9 surrounding pixels
	Direction findDirection(const Coord& p) const;

	// Return reference to the matrix for walking edge in a direction
	Coord matrix(const Coord& p, int index, Direction dir) const;
};

#endif
