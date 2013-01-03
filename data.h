/*
 * Data stuff that needs to be used most places
 */

#ifndef H_DATA
#define H_DATA

#include <iostream>

struct Coord
{
	int x = 0;
	int y = 0;

	Coord() { }
	Coord(const int x, const int y) :x(x), y(y) { }

	Coord& operator+=(const Coord& c);
	Coord operator+(const Coord& c) const;
};

std::ostream& operator<<(std::ostream& os, const Coord& c);
std::istream& operator>>(std::istream& is, Coord& c);
bool operator==(const Coord& c1, const Coord& c2);
bool operator!=(const Coord& c1, const Coord& c2);

// Less than: Is the y value less? If the same, is the x value less?
bool operator<(const Coord& c1, const Coord& c2);
bool operator>(const Coord& c1, const Coord& c2);

#endif
