#include "rotate.h"

// Determine if this pixel is in a box using diagonal from left to right corner and
// seeing if the center is black and return the diagonal
BoxData analyzeBox(Pixels& img, const Coord& p,
	const unsigned int& max_x, const unsigned int& max_y)
{
	bool is_box = false;
	Coord left      = leftmost(img,  p, max_x, max_y);
	Coord right     = rightmost(img, p, max_x, max_y);
	Coord midpoint  = midPoint(left, right);
	double diagonal = distance(left, right);

	// The "diagonal" should be at minimum the box width (if both the same y value) and
	// at max the diagonal. See if the center of the box is black.
	if (diagonal <= DIAGONAL+MAX_ERROR && diagonal >= BOX_WIDTH-MAX_ERROR &&
		averageColor(img, midpoint.x, midpoint.y, BOX_HEIGHT/2, max_x, max_y) > MIN_BLACK)
		is_box = true;
	
	return BoxData(diagonal, is_box);
}

// Find top left and bottom right box. Then, determine slope of these two
// and return the amount to rotate.
double findRotation(Pixels& img, Coord& ret_coord,
	const unsigned int& max_x, const unsigned int& max_y)
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
			if (isBlack(img, x, y))
			{
				Coord point(x, y);
				BoxData data = analyzeBox(img, point, max_x, max_y);

				if (data.is_box)
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
				if (data.diagonal > DECENT_SIZE)
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
				Coord point(x, y);
				BoxData data = analyzeBox(img, point, max_x, max_y);

				if (data.is_box)
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
				if (data.diagonal > DECENT_SIZE)
					break;
			}
		}
	}

	// Determine angle from slope of line going through those two boxes
	double angle = 0;
	ret_coord = top;

	/*image.fillColor("pink");
	image.draw(DrawableRectangle(top.x-5, top.y-5,
		top.x+5, top.y+5));
	image.draw(DrawableRectangle(bottom.x-5, bottom.y-5,
		bottom.x+5, bottom.y+5));*/
	
	// If denominator is zero, don't rotate
	if (top.y != bottom.y)
		angle = atan((1.0*bottom.x - top.x)/(1.0*bottom.y - top.y));

	return angle;
}
