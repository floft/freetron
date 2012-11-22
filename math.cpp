#include "math.h"

// Standard Deviation:
//   sqrt(1/n*((x1 - avg)^2 + (x2 - avg)^2 + (xn - avg)^2))
double stdDev(vector<double> v, double& average)
{
	if (v.size() == 0)
		return 0;

	double inroot = 0;
	double total  = 0;
	double mean   = 0;

	for (unsigned int i = 0; i < v.size(); ++i)
		total += v[i];

	mean = total/v.size();

	for (unsigned int i = 0; i < v.size(); ++i)
		inroot += pow(mean - v[i], 2);
	
	average = mean;
	
	return sqrt(inroot/v.size());
}

// Distance formula
double distance(double x1, double y1, double x2, double y2)
{
	return sqrt(pow((x2-x1), 2) + pow((y2-y1), 2));
}

double distance(Coordinate p1, Coordinate p2)
{
	return sqrt(pow((p2.x()-p1.x()), 2) + pow((p2.y()-p1.y()), 2));
}

// Round x to the nearest r
//   see: http://stackoverflow.com/a/3407254
unsigned int round(const unsigned int& x, const unsigned int& r)
{
	int remainder = x%r;

	if (remainder == 0)
		return x;
	
	return x + r - remainder;
}
