#include "data.h"

std::ostream& operator<<(std::ostream& os, const Coord& c)
{
	return os << "(" << c.x << "," << c.y << ")";
}

std::istream& operator>>(std::istream& is, Coord& c)
{
	is.exceptions(is.exceptions()|std::ios_base::badbit);

	char paren1, comma, paren2;

	is >> paren1 >> c.x >> comma >> c.y >> paren2;

	return is;
}

bool operator==(const Coord& c1, const Coord& c2)
{
	return (c1.x == c2.x && c1.y == c2.y);
}

bool operator!=(const Coord& c1, const Coord& c2)
{
	return !(c1==c2);
}
