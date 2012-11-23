/*
 * Math functions
 */

#ifndef H_MATHFUNCTIONS
#define H_MATHFUNCTIONS

#include <cmath>
#include <vector>
#include <algorithm>

#include "data.h"

using namespace std;

const double pi = 3.14159265358979323846264338327950;

double stdDev(vector<double> v, double& average);
double distance(double x1, double y1, double x2, double y2);
double distance(Coord p1, Coord p2);
double average(vector<double> v);
double max_value(vector<double> v);
unsigned int round(const unsigned int& x, const unsigned int& r);

#endif
