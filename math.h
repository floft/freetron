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

inline double distance(const double x1, const double y1, const double x2, const double y2);
inline double distance(const Coord& p1, const Coord& p2);
inline double average(const std::vector<double>& v);
inline double max_value(const std::vector<double>& v);
inline double min_value(const std::vector<double>& v);
inline int round(const int x, const int r);
inline int smartFloor(const double value, const double epsilon = 0.00001);
inline int smartCeil(const double value,  const double epsilon = 0.00001);
inline int lineFunctionX(const Coord& a, const Coord& b, int y);
inline int lineFunctionY(const Coord& a, const Coord& b, int x);
inline double slopeYX(const Coord& a, const Coord& b);
inline double slopeXY(const Coord& a, const Coord& b);

// Standard Deviation:
//   sqrt(1/n*((x1 - avg)^2 + (x2 - avg)^2 + ... (xn - avg)^2))
//
// Type should probably be std::vector<double>
template<class Type>
double stdDev(const Type& v)
{
	if (v.size() == 0)
		return 0;

	double total = 0;
	std::accumulate(v.begin(), v.end(), total);

	double mean = total/v.size();
	double inroot = 0;

	for (const double elem : v)
		inroot += std::pow(mean - elem, 2);
	
	return std::sqrt(inroot/v.size());
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

// Ceil but after subtracting a bit
inline int smartCeil(const double value, const double epsilon)
{
	return std::ceil(value - epsilon);
}

// y = m(x-x1)+y1
inline int lineFunctionY(const Coord& a, const Coord& b, int x)
{
	return smartFloor(slopeYX(a,b)*(x - a.x) + a.y);
}

// x = (y-y1)/m+x1
inline int lineFunctionX(const Coord& a, const Coord& b, int y)
{
	return smartFloor(slopeXY(a,b)*(y - a.y) + a.x);
}

// Rise over run
inline double slopeYX(const Coord& a, const Coord& b)
{
	if (b.x - a.x == 0)
		return 0;
	else
		return 1.0*(b.y - a.y)/(b.x - a.x);
}

// Run over rise
inline double slopeXY(const Coord& a, const Coord& b)
{
	if (b.y - a.y == 0)
		return 0;
	else
		return 1.0*(b.x - a.x)/(b.y - a.y);
}

#endif
