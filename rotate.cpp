#include "rotate.h"

// Find top left and bottom right box. Then, determine slope of these two
// and return the amount to rotate.
double findRotation(Pixels& img, Coord& ret_coord,
	const unsigned int max_x, const unsigned int max_y, BoxData* box_data)
{
	Coord top;
	Coord bottom;

	// Set to max large values
	double min_top_dist    = max_y;
	double min_bottom_dist = max_y;

	// Top and bottom points
	const Coord origin(0, 0);
	const Coord extreme(0, max_y - 1);

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
			if (img.black(Coord(x, y)))
			{
				Coord point(x, y);
				Box box(img, point, max_x, max_y, box_data);

				if (box.valid())
				{
					double current_top_dist = distance(point, origin);

					// See if this box is closer to the top left than the previous one
					if (current_top_dist < min_top_dist)
					{
						min_top_dist = current_top_dist;
						top = point;
					}
					// We are getting farther away, so we already found the closest box
					else if (current_top_dist - min_top_dist > MIN_JUMP)
					{
						found = true;
					}
				}
				
				// We only care about the left-most black blob, skip if this is a decent-sized blob
				if (box.width() > DECENT_SIZE)
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
			if (img.black(Coord(x, y)))
			{
				Coord point(x, y);
				Box box(img, point, max_x, max_y, box_data);

				if (box.valid())
				{
					double current_bottom_dist = distance(point, extreme);

					// It's closer than the previous box
					if (current_bottom_dist < min_bottom_dist)
					{
						min_bottom_dist = current_bottom_dist;
						bottom = point;
					}
					// We're starting to get farther away, so we probably found the closest point
					else if (current_bottom_dist - min_bottom_dist > MIN_JUMP)
					{
						found = true;
					}
				}
				
				// We only care about the left-most black blob, skip if this is a decent-sized blob
				if (box.width() > DECENT_SIZE)
					break;
			}
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
