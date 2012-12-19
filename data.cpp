#include "data.h"

std::ostream& operator<<(std::ostream& os, const Coord& c)
{
	return os << "(" << c.x << "," << c.y << ")";
}

bool operator==(const Coord& c1, const Coord& c2)
{
	return (c1.x == c2.x && c1.y == c2.y);
}

bool operator!=(const Coord& c1, const Coord& c2)
{
	return !(c1==c2);
}
