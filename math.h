/*
 * Math functions
 */

#ifndef H_MATHFUNCTIONS
#define H_MATHFUNCTIONS

#include <cmath>
#include <vector>
#include <algorithm>

#include "data.h"

static const double pi = 3.14159265358979323846264338327950;

double stdDev(const std::vector<double>& v, double& average);
inline double distance(const double x1, const double y1, const double x2, const double y2);
inline double distance(const Coord& p1, const Coord& p2);
inline double average(const std::vector<double>& v);
inline double max_value(const std::vector<double>& v);
inline double min_value(const std::vector<double>& v);
inline int round(const int x, const int r);
inline int smartFloor(const double value, const double epsilon = 0.00001);
inline int lineFunctionX(const Coord& a, const Coord& b, int y);
inline int lineFunctionY(const Coord& a, const Coord& b, int x);

// Two functions that allow a simple mapMinValue<Coord>(start, end) returning
// the value of the minimum key in the map. Example:
//  Coord c = mapMinValue<Coord, std::pair<double, Coord>>(m.begin(), m.end())
template<class T>
class mapKeyCompare
{
public:
	bool operator()(const T& a, const T& b)
	{
		return (a.first < b.first);
	}
};

template<class Result, class Type, class Iter>
Result mapMinValue(Iter start, Iter end)
{
	Iter min = std::min_element(start, end, mapKeyCompare<Type>());

	if (min != end)
		return min->second;
	else
		return Result();
}

/*
 * Inline stuff since these are tiny
 */

// Distance formula
inline double distance(const double x1, const double y1, const double x2, const double y2)
{
	return std::sqrt(std::pow((x2-x1), 2) + std::pow((y2-y1), 2));
}

inline double distance(const Coord& p1, const Coord& p2)
{
	return std::sqrt(std::pow((1.0*p2.x-p1.x), 2) + std::pow((1.0*p2.y-p1.y), 2));
}

// Calculate mean
inline double average(const std::vector<double>& v)
{
	if (v.size() > 0)
		return std::accumulate(v.begin(), v.end(), 0.0)/v.size();
	
	return 0;
}

// Check for blank vector
inline double max_value(const std::vector<double>& v)
{
	if (v.size() > 0)
		return *std::max_element(v.begin(), v.end());
	
	return 0;
}

// Check for blank vector
inline double min_value(const std::vector<double>& v)
{
	if (v.size() > 0)
		return *std::min_element(v.begin(), v.end());
	
	return 0;
}

// Round x to the nearest r
//   see: http://stackoverflow.com/a/3407254
inline int round(const int x, const int r)
{
	int remainder = x%r;

	if (remainder == 0)
		return x;

	return x + r - remainder;
}

// Floor but after adding a bit
inline int smartFloor(const double value, const double epsilon)
{
	return std::floor(value + epsilon);
}

// y = m(x-x1)+y1
inline int lineFunctionY(const Coord& a, const Coord& b, int x)
{
	if (b.x - a.x == 0)
		return 0;
	else    
		return smartFloor(1.0*(b.y - a.y)/(b.x - a.x)*(x - a.x) + a.y);
}

// x = (y-y1)/m+x1
inline int lineFunctionX(const Coord& a, const Coord& b, int y)
{
	if (b.y - a.y == 0)
		return 0;
	else
		return smartFloor(1.0*(b.x - a.x)/(b.y - a.y)*(y - a.y) + a.x);
}

#endif
