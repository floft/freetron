/*
 * Data stuff that needs to be used most places
 */

#ifndef H_DATA
#define H_DATA

#include <iostream>

struct Coord
{
	int x;
	int y;

	Coord() :x(0), y(0) { }
	Coord(const int x, const int y) :x(x), y(y) { }
};

std::ostream& operator<<(std::ostream& os, const Coord& c);
std::istream& operator>>(std::istream& is, Coord& c);
bool operator==(const Coord& c1, const Coord& c2);
bool operator!=(const Coord& c1, const Coord& c2);

#endif
