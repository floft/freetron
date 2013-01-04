#include "rotate.h"

// Find top left and bottom right box. Then, determine slope of these two
// and return the amount to rotate.
double findRotation(const Pixels& img, const std::vector<Coord>& boxes, Coord& ret_coord)
{
	if (boxes.size() < ROTATE_TOP || boxes.size() < ROTATE_BOTTOM)
		return 0;

	Coord top = boxes[ROTATE_TOP];
	Coord bottom = boxes[ROTATE_BOTTOM];

	// Determine angle from slope of line going through those two boxes
	double angle = 0;
	ret_coord = top;
	
	// If denominator is zero, don't rotate
	if (top.y != bottom.y)
		angle = std::atan((1.0*bottom.x - top.x)/(1.0*bottom.y - top.y));

	return angle;
}
