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
};

struct BoxData
{
	double diagonal;
	bool   is_box;

	BoxData() :diagonal(0), is_box(false) { }

	BoxData(const double& diagonal, const bool& is_box)
		:diagonal(diagonal), is_box(is_box) { }
};

ostream& operator<<(ostream& os, const Coord& c);
bool operator==(const Coord& c1, const Coord& c2);
bool operator!=(const Coord& c1, const Coord& c2);

#endif
