/*
 * Math functions
 */

#ifndef H_MATHFUNCTIONS
#define H_MATHFUNCTIONS

#include <cmath>
#include <vector>
#include <algorithm>

#include <Magick++.h>

using namespace std;
using namespace Magick;

const double pi = 3.14159265358979323846264338327950;

double stdDev(vector<double> v, double& average);
double distance(double x1, double y1, double x2, double y2);
double distance(Coordinate p1, Coordinate p2);
unsigned int round(const unsigned int& x, const unsigned int& r);

#endif
