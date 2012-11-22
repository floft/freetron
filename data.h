/*
 * Data stuff that needs to be used most places
 */

#ifndef H_DATA
#define H_DATA

struct Coord {
	unsigned int x;
	unsigned int y;

	Coord() :x(0), y(0) { }
	Coord(const unsigned int& x, const unsigned int& y) :x(x), y(y) { }
};

#endif
