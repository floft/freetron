/*
 * Math functions
 */

#ifndef H_MATHFUNCTIONS
#define H_MATHFUNCTIONS

#include <cmath>
#include <vector>
#include <algorithm>

#include "data.h"

const double pi = 3.14159265358979323846264338327950;

double stdDev(const std::vector<double>& v, double& average);
double distance(const double x1, const double y1, const double x2, const double y2);
double distance(const Coord& p1, const Coord& p2);
double average(const std::vector<double>& v);
double max_value(const std::vector<double>& v);
double min_value(const std::vector<double>& v);
int round(const int x, const int r);
int smartFloor(const double value, const double epsilon = 0.00001);
int lineFunctionX(const Coord& a, const Coord& b, int y);
int lineFunctionY(const Coord& a, const Coord& b, int x);

// Two functions that allow a simple mapMinValue<Coord>(start, end) returning
// the value of the minimum key in the map
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

#endif
