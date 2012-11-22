#include "rotate.h"

// Find top left and bottom right box. Then, determine slope of these two
// and return the amount to rotate.
double findRotation(Pixels& img, unsigned int& ret_x, unsigned int& ret_y,
	const unsigned int& max_x, const unsigned int& max_y)
{
	Coord top;
	Coord bottom;

	// Set to max large values
	double min_top_dist    = max_y;
	double min_bottom_dist = max_y;

	//
	// The top-left box
	//

	// Goto statements are evil
	bool found = false;

	// Search from top left up a y = x line going down the image
	// Max y+x for also scanning the bottom of the image if shifted to the right
	for (unsigned int z = 0; z < max_y + max_x && !found; ++z)
	{
		for (unsigned int x = 0, y = z; x <= z && x < max_x && !found; ++x, --y)
		{
			// This is an imaginary point (skip till we get to points on the
			// bottom of the image)
			if (y > max_y - 1)
				continue;

			// See if it might be a box
			if (isBlack(img, x, y))
			{
				Coord left      = leftmost(img,  x, y, max_x, max_y);
				Coord right     = rightmost(img, x, y, max_x, max_y);
				Coord midpoint  = midPoint(left, right);
				double diagonal = distance(left, right);

				// See if the diagonal is about the right length and if a circle in the center
				// of the possible box is almost entirely black
				if (diagonal <= DIAGONAL+MAX_ERROR && diagonal >= DIAGONAL-MAX_ERROR &&
					averageColor(img, midpoint.x, midpoint.y, BOX_HEIGHT/2, max_x, max_y) > MIN_BLACK)
				{
					double current_top_dist = distance(x, y, 0, 0);

					// See if this box is closer to the top left than the previous one
					if (current_top_dist < min_top_dist)
					{
						min_top_dist = current_top_dist;
						top = Coord(x, y);
					}
					// We are getting farther away, so we already found the closest box
					else if (current_top_dist - min_top_dist > MIN_JUMP)
					{
						found = true;
					}
				}
				
				// We only care about the left-most black blob, skip if this is a decent-sized blob
				if (diagonal > DECENT_SIZE)
					break;
			}
		}
	}

	// 
	// The bottom-left box
	//
	found = false;

	// Start searching at the bottom
	// Stop searching once reaching the y value of the top-left box
	for (unsigned int z = max_y + max_x; z > top.y && !found; --z)
	{
		for (unsigned int x = 0, y = z; x <= z && x < max_x && !found; ++x, --y)
		{
			// This is an imaginary point (below the bottom)
			if (y > max_y - 1)
				continue;

			// It's black
			if (isBlack(img, x, y))
			{
				Coord left      = leftmost(img,  x, y, max_x, max_y);
				Coord right     = rightmost(img, x, y, max_x, max_y);
				Coord midpoint  = midPoint(left, right);
				double diagonal = distance(left, right);

				// It's a box
				if (diagonal <= DIAGONAL+MAX_ERROR && diagonal >= DIAGONAL-MAX_ERROR &&
					averageColor(img, midpoint.x, midpoint.y, BOX_HEIGHT/2, max_x, max_y) > MIN_BLACK)
				{

					double current_bottom_dist = distance(left.x, left.y, 0, max_y - 1);

					// It's closer than the previous box
					if (current_bottom_dist < min_bottom_dist)
					{
						min_bottom_dist = current_bottom_dist;
						bottom = Coord(left.x, left.y);
					}
					// We're starting to get farther away, so we probably found the closest point
					else if (current_bottom_dist - min_bottom_dist > MIN_JUMP)
					{
						found = true;
					}
				}
				
				// We only care about the left-most black blob, skip if this is a decent-sized blob
				if (diagonal > DECENT_SIZE)
					break;
			}
		}
	}

	// Determine angle from slope of line going through those two boxes
	unsigned int x1 = top.x;
	unsigned int y1 = top.y;
	unsigned int x2 = bottom.x;
	unsigned int y2 = bottom.y;

	ret_x = top.x;
	ret_y = top.y;

	double angle;
	
	// If denominator is zero, don't rotate
	if (y1 != y2)
		angle = atan((1.0*x2 - x1)/(1.0*y2 - y1));
	else
		angle = 0;

	return angle;
}
