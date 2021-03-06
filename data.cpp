#include "data.h"

std::ostream& operator<<(std::ostream& os, const Coord& c)
{
    return os << "(" << c.x << "," << c.y << ")";
}

std::ostream& operator<<(std::ostream& os, const Answer& c)
{
    switch (c)
    {
        case Answer::Blank: return os << "_";
        case Answer::A:     return os << "A";
        case Answer::B:     return os << "B";
        case Answer::C:     return os << "C";
        case Answer::D:     return os << "D";
        case Answer::E:     return os << "E";
        default:            return os << "?";
    }
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

bool operator<(const Coord& c1, const Coord& c2)
{
    return (c1.y < c2.y || (c1.y == c2.y && c1.x < c2.x));
}

bool operator>(const Coord& c1, const Coord& c2)
{
    return !(c1 < c2) && !(c1 == c2);
}

Coord Coord::operator+(const Coord& c) const
{
    return Coord(x+c.x, y+c.y);
}

Coord& Coord::operator+=(const Coord& c)
{
    x += c.x;
    y += c.y;

    return *this;
}
