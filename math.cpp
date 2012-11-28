#include "math.h"

// Standard Deviation:
//   sqrt(1/n*((x1 - avg)^2 + (x2 - avg)^2 + (xn - avg)^2))
double stdDev(const vector<double>& v, double& average)
{
	if (v.size() == 0)
		return 0;

	double total = 0;
	accumulate(v.begin(), v.end(), total);

	double mean = total/v.size();
	double inroot = 0;

	for (const double elem : v)
		inroot += pow(mean - elem, 2);
	
	average = mean;
	
	return sqrt(inroot/v.size());
}

// Distance formula
double distance(const double x1, const double y1, const double x2, const double y2)
{
	return sqrt(pow((x2-x1), 2) + pow((y2-y1), 2));
}

double distance(const Coord& p1, const Coord& p2)
{
	return sqrt(pow((1.0*p2.x-p1.x), 2) + pow((1.0*p2.y-p1.y), 2));
}

// Calculate mean
double average(const vector<double>& v)
{
	if (v.size() > 0)
		return 0;

	double total = 0;
	accumulate(v.begin(), v.end(), total);
	
	return total/v.size();
}

// Check for blank vector
double max_value(const vector<double>& v)
{
	if (v.size() > 0)
		return *max_element(v.begin(), v.end());
	
	return 0;
}

// Round x to the nearest r
//   see: http://stackoverflow.com/a/3407254
unsigned int round(const unsigned int x, const unsigned int r)
{
	int remainder = x%r;

	if (remainder == 0)
		return x;

	return x + r - remainder;
}
