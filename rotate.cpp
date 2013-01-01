#include "rotate.h"

// Find top left and bottom right box. Then, determine slope of these two
// and return the amount to rotate.
double findRotation(Pixels& img, const std::vector<Coord>& boxes, Coord& ret_coord, BoxData* data)
{
	Coord top;
	Coord bottom;

	// Set to max large values
	double min_top_dist    = img.height();
	double min_bottom_dist = img.height();

	// Top and bottom points
	const Coord origin(0, 0);
	const Coord extreme(0, img.height() - 1);

	// Find closest box to top and bottom left
	for (const Coord& c : boxes)
	{
		double dist_origin  = distance(c, origin);
		double dist_extreme = distance(c, extreme);

		if (dist_origin < min_top_dist)
		{
			top = c;
			min_top_dist = dist_origin;
		}

		if (dist_extreme < min_bottom_dist)
		{
			bottom = c;
			min_bottom_dist = dist_extreme;
		}
	}

	// Determine angle from slope of line going through those two boxes
	double angle = 0;
	ret_coord = top;
	
	// If denominator is zero, don't rotate
	if (top.y != bottom.y)
		angle = std::atan((1.0*bottom.x - top.x)/(1.0*bottom.y - top.y));

	return angle;
}
