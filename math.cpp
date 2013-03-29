#include "math.h"

Coord farthestFromPoint(const Coord& p, const std::vector<Coord>& points)
{
    double dist = 0;
    Coord farthest;

    for (const Coord& point : points)
    {
        double cdist = distance(p, point);

        if (cdist > dist)
        {
            dist = cdist;
            farthest = point;
        }
    }

    return farthest;
}

Coord farthestFromLine(const Coord& p1, const Coord& p2,
    const std::vector<Coord>& points)
{
    double dist = 0;
    Coord farthest;

    // Only look at the distance in x if the line is vertical
    if (p2.x == p1.x)
    {
        for (const Coord& point : points)
        {
            double cdist = std::abs(point.x - p1.x);

            if (cdist > dist)
            {
                dist = cdist;
                farthest = point;
            }
        }
    }
    else
    {
        // We'll redefine this here instead of using distance(p1, p2, p3) since
        // we only need to calculate m, b, and sq once
        double m = 1.0*(p2.y - p1.y)/(p2.x - p1.x);
        double b = p1.y - m*p1.x;
        double sq = std::sqrt(m*m + 1);

        for (const Coord& point : points)
        {
            // Perpendicular distance: abs(mx-y+b)/sqrt(m^2+1)
            double cdist = std::abs(m*point.x - point.y + b)/sq;

            if (cdist > dist)
            {
                dist = cdist;
                farthest = point;
            }
        }
    }

    return farthest;
}

// Find the center. We move the origin to the first point so that the total x and y
// values will fit in an int even at the far edges of the image. Then, later
// we move it back to the proper spot.
Coord findCenter(const std::vector<Coord>& points)
{
    if (points.size() == 0)
        return default_coord;

    int x = 0;
    int y = 0;

    for (const Coord& p : points)
    {
        x += p.x-points[0].x;
        y += p.y-points[0].y;
    }

    return Coord(1.0*x/points.size()+points[0].x,
                 1.0*y/points.size()+points[0].y);
}
