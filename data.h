/*
 * Data stuff that needs to be used most places
 */

#ifndef H_DATA
#define H_DATA

#include <iostream>

using namespace std;

struct Coord
{
	unsigned int x;
	unsigned int y;

	Coord() :x(0), y(0) { }
	Coord(const unsigned int& x, const unsigned int& y) :x(x), y(y) { }

	// Show this on the image, remember to do img.sync()
	//void display(Pixels& img, Color c) const { *(img.get(x, y, 1, 1)) = c; }
};

struct Options
{
	unsigned int first_jump;	// Jump to first bubble from black box
	unsigned int bubble_jump;	// Jump from one bubble to the next
	double black;			// Percent (0-1) of pixels for marking answer
	double rotation;		// Rotation needed to get boxes vertical
	Coord rotate_point;		// Rotate around this point
};

ostream& operator<<(ostream& os, const Coord& c);
bool operator==(const Coord& c1, const Coord& c2);
bool operator!=(const Coord& c1, const Coord& c2);

#endif
