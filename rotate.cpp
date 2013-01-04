#include "rotate.h"

// Find top left and bottom right box. Then, determine slope of these two
// and return the amount to rotate.
/*double findRotation(Pixels& img, Coord& ret_coord, BoxData* data)
{
	Coord top;
	Coord bottom;

	// Set to max large values
	double min_top_dist    = img.height();
	double min_bottom_dist = img.height();

	// Top and bottom points
	const Coord origin(0, 0);
	const Coord extreme(0, img.height() - 1);

	//
	// The top-left box
	//

	// Goto statements are evil
	bool found = false;

	// Search from top left up a y = x line going down the image
	// Max y+x for also scanning the bottom of the image if shifted to the right
	for (int z = 0; z < img.height() + img.width() && !found; ++z)
	{
		for (int x = 0, y = z; x <= z && x < img.width() && !found; ++x, --y)
		{
			// This is an imaginary point (skip till we get to points on the
			// bottom of the image)
			if (y > img.height() - 1)
				continue;

			// See if it might be a box
			if (img.black(Coord(x, y)))
			{
				Coord point(x, y);
				Box box(&img, point, data);

				if (box.valid())
				{
					double current_top_dist = distance(box.midpoint(), origin);

					// See if this box is closer to the top left than the previous one
					if (current_top_dist < min_top_dist)
					{
						min_top_dist = current_top_dist;
						top = box.midpoint();
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
	for (int z = img.height() + img.width(); z > top.y && !found; --z)
	{
		for (int x = 0, y = z; x <= z && x < img.width() && !found; ++x, --y)
		{
			// This is an imaginary point (below the bottom)
			if (y > img.height() - 1)
				continue;

			// It's black
			if (img.black(Coord(x, y)))
			{
				Coord point(x, y);
				Box box(&img, point, data);

				if (box.valid())
				{
					double current_bottom_dist = distance(box.midpoint(), extreme);

					// It's closer than the previous box
					if (current_bottom_dist < min_bottom_dist)
					{
						min_bottom_dist = current_bottom_dist;
						bottom = box.midpoint();
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

	//img.mark(top);
	//img.mark(bottom);

	// Determine angle from slope of line going through those two boxes
	double angle = 0;
	ret_coord = top;
	
	// If denominator is zero, don't rotate
	if (top.y != bottom.y)
		angle = std::atan((1.0*bottom.x - top.x)/(1.0*bottom.y - top.y));

	return angle;
}*/

double findRotation(Pixels& img, Coord& ret_coord)
{
	Coord start;
	Coord end;
	int length = 0;
	double angle = 0;
	bool found = false;

	// We will never need to deal with the first pixels of each row/column
	for (int y = 1; y < img.height() && !found; ++y)
	{
		for (int x = 1; x < img.width() && !found; ++x)
		{
			Coord current(x, y);

			if (img.black(current))
			{
				Coord point = findBottom(img, current);
				int dist    = distance(current, point);

				if (current == Coord(484, 198))
				{
					std::cout << current << " " << point << " " << dist << std::endl;
					img.mark(point);
					img.mark(current);
				}

				if (dist >= ROTATE_LEN)
				{
					start = current;
					end   = point;
					img.mark(start);
					img.mark(end);
					found = true;
				}
				else if (dist >= DECENT_SIZE)
				{
					break;
				}
			}
		}
	}
	
	if (found && start.y != end.y)
		angle = std::atan((1.0*end.x - start.x)/(1.0*end.y - start.y));
	
	return angle;
}

// Go up till $error pixels, return new y
int goUp(const Pixels& img, const Coord& p, const Coord& orig)
{
	int new_y = p.y;
	int white_count = 0;

	// Go until $error white pixels, hit top of image, or diag
	// is greater than possible for a box.
	for (int search_y = p.y; search_y >= 0 && white_count <= MAX_ERROR &&
		distance(p.x, search_y, orig.x, orig.y) <= ROTATE_LEN; --search_y)
	{
		if (img.black(Coord(p.x, search_y)))
		{
			new_y = search_y;
			white_count = 0;
		}
		else
		{
			++white_count;
		}
	}
	
	return new_y;
}

int goLeft(const Pixels& img, const Coord& p, const Coord& orig)
{
	int new_x = p.x;
	int white_count = 0;

	for (int search_x = p.x; search_x >= 0 && white_count <= MAX_ERROR &&
		distance(search_x, p.y, orig.x, orig.y) <= ROTATE_LEN; --search_x)
	{
		if (img.black(Coord(search_x, p.y)))
		{
			new_x = search_x;
			white_count = 0;
		}
		else
		{
			++white_count;
		}
	}
	
	return new_x;
}

int goDown(const Pixels& img, const Coord& p, const Coord& orig)
{
	int new_y = p.y;
	int white_count = 0;

	for (int search_y = p.y; search_y < img.height() && white_count <= MAX_ERROR &&
		distance(p.x, search_y, orig.x, orig.y) <= ROTATE_LEN; ++search_y)
	{
		if (img.black(Coord(p.x, search_y)))
		{
			new_y = search_y;
			white_count = 0;
		}
		else
		{
			++white_count;
		}
	}
	
	return new_y;
}

int goRight(const Pixels& img, const Coord& p, const Coord& orig)
{
	int new_x = p.x;
	int white_count = 0;

	for (int search_x = p.x; search_x < img.width() && white_count <= MAX_ERROR &&
		distance(search_x, p.y, orig.x, orig.y) <= ROTATE_LEN; ++search_x)
	{
		if (img.black(Coord(search_x, p.y)))
		{
			new_x = search_x;
			white_count = 0;
		}
		else
		{
			++white_count;
		}
	}
	
	return new_x;
}

// Find the bottommost coordinate of a box
Coord findBottom(const Pixels& img, const Coord& point)
{
	Coord bottom(point.x, goDown(img, point, point));

	// Go left and right, find midpoint. Go down. If we can't move, we found it.
	while (distance(point, bottom) <= ROTATE_LEN)
	{
		bottom.x = (goLeft(img, bottom, point) + goRight(img, bottom, point))/2;

		int bottom_y = goDown(img, bottom, point);

		if (bottom_y == bottom.y)
			break;

		bottom.y = bottom_y;
	}

	return bottom;
}
