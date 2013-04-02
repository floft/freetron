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
// Perpendicular distance between p3 and the line from p1 to p2
inline double distance(const Coord& p1, const Coord& p2, const Coord& p3);
inline double average(const std::vector<double>& v);
inline double average(const std::vector<double>& v,
    std::vector<double>::const_iterator start,
    std::vector<double>::const_iterator end);
inline double max_value(const std::vector<double>& v);
inline double min_value(const std::vector<double>& v);
inline double min(double, double, double);
inline double max(double, double, double);
inline int round(const int x, const int r);
inline int smartFloor(const double value, const double epsilon = 0.00001);
inline int smartCeil(const double value,  const double epsilon = 0.00001);
inline int lineFunctionX(const Coord& a, const Coord& b, int y);
inline int lineFunctionY(const Coord& a, const Coord& b, int x);
inline double slopeYX(const Coord& a, const Coord& b);
inline double slopeXY(const Coord& a, const Coord& b);
inline Coord findMidpoint(const Coord& a, const Coord& b);

// Used to find corners of boxes and bubbles
Coord farthestFromPoint(const Coord& p,
     const std::vector<Coord>& points);
Coord farthestFromLine(const Coord& p1, const Coord& p2,
     const std::vector<Coord>& points);

// Determine "center" by averaging all points
Coord findCenter(const std::vector<Coord>& points);

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

// Perpendicular distance
inline double distance(const Coord& p1, const Coord& p2, const Coord& p3)
{
    // Only look at horizontal distance if line is vertical
    if (p2.x == p1.x)
    {
        return std::abs(p1.x - p3.x);
    }
    else
    {
        // Find the equation for the line in the form y = mx + b
        double m = 1.0*(p2.y - p1.y)/(p2.x - p1.x);
        double b = p1.y - m*p1.x;
        return std::abs(m*p3.x - p3.y + b)/std::sqrt(m*m + 1);
    }
}

// Calculate mean
inline double average(const std::vector<double>& v)
{
    if (v.size() > 0)
        return std::accumulate(v.begin(), v.end(), 0.0)/v.size();
    
    return 0;
}

inline double average(const std::vector<double>& v,
    std::vector<double>::const_iterator start,
    std::vector<double>::const_iterator end)
{
    if (v.size() > 0)
        return std::accumulate(start, end, 0.0)/v.size();
    
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

inline Coord findMidpoint(const Coord& a, const Coord& b)
{
    return Coord((a.x+b.x)/2, (a.y+b.y)/2);
}

// 3 way compares
inline double min(double a, double b, double c)
{
    double m = a;
    
    if (m > b) m = b;
    if (m > c) m = c;

    return m;
}

inline double max(double a, double b, double c)
{
    double m = a;

    if (m < b) m = b;
    if (m < c) m = c;

    return m;
}

#endif
