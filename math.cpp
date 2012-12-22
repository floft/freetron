#include "math.h"

// Standard Deviation:
//   sqrt(1/n*((x1 - avg)^2 + (x2 - avg)^2 + (xn - avg)^2))
double stdDev(const std::vector<double>& v, double& average)
{
	if (v.size() == 0)
		return 0;

	double total = 0;
	std::accumulate(v.begin(), v.end(), total);

	double mean = total/v.size();
	double inroot = 0;

	for (const double elem : v)
		inroot += std::pow(mean - elem, 2);
	
	average = mean;
	
	return std::sqrt(inroot/v.size());
}

// Distance formula
double distance(const double x1, const double y1, const double x2, const double y2)
{
	return std::sqrt(std::pow((x2-x1), 2) + std::pow((y2-y1), 2));
}

double distance(const Coord& p1, const Coord& p2)
{
	return std::sqrt(std::pow((1.0*p2.x-p1.x), 2) + std::pow((1.0*p2.y-p1.y), 2));
}

// Calculate mean
double average(const std::vector<double>& v)
{
	if (v.size() > 0)
		return std::accumulate(v.begin(), v.end(), 0.0)/v.size();
	
	return 0;
}

// Check for blank vector
double max_value(const std::vector<double>& v)
{
	if (v.size() > 0)
		return *std::max_element(v.begin(), v.end());
	
	return 0;
}

// Check for blank vector
double min_value(const std::vector<double>& v)
{
	if (v.size() > 0)
		return *std::min_element(v.begin(), v.end());
	
	return 0;
}

// Round x to the nearest r
//   see: http://stackoverflow.com/a/3407254
int round(const int x, const int r)
{
	int remainder = x%r;

	if (remainder == 0)
		return x;

	return x + r - remainder;
}

// Floor but after adding a bit
int smartFloor(const double value, const double epsilon)
{
	return std::floor(value + epsilon);
}
