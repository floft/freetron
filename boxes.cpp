#include "boxes.h"

// Sort based on y values of top-left points
bool box_sort(const Coord& v1, const Coord& v2)
{
	return (v1.y < v2.y);
}

// Find all the boxes in the image
vector<Coord> findBoxes(Pixels& img,
	const unsigned int& max_x, const unsigned int& max_y)
{
	vector<Coord> boxes;

	// Find all the boxes searching from down the image going up at a diagonal to the
	// top for each y value. The max_y+max_x also scans coming up from the bottom of
	// the image.
	for (unsigned int z = 0; z < max_y + max_x; ++z)
	{
		for (unsigned int x = 0, y = z; x <= z && x < max_x; ++x, --y)
		{
			// This is an imaginary point (skip till we get to points on the
			// bottom of the image)
			if (y > max_y - 1)
				continue;

			// See if it might be a box
			if (isBlack(img, x, y))
			{
				Coord point(x, y);
				Coord left      = leftmost(img,   point, max_x, max_y);
				Coord right     = rightmost(img,  point, max_x, max_y);
				Coord top       = topmost(img,    point, max_x, max_y);
				Coord bottom    = bottommost(img, point, max_x, max_y);
				Coord midpoint((left.x + right.x)/2, (top.y + bottom.y)/2);

				unsigned int height = bottom.y - top.y;
				unsigned int width  = right.x  - left.x;

				// See if the diagonal is about the right length, if the width and height are about right,
				// and if a circle in the center of the possible box is almost entirely black.
				if (abs(1.0*width - BOX_WIDTH) <= MAX_ERROR && abs(1.0*height - BOX_HEIGHT) <= MAX_ERROR &&
					averageColor(img, midpoint.x, midpoint.y, BOX_HEIGHT/2, max_x, max_y) > MIN_BLACK)
				{
					// Make sure we didn't already have this point
					if (find(boxes.begin(), boxes.end(), midpoint) == boxes.end())
						boxes.push_back(midpoint);
				}
				
				// We only care about the left-most black blob, skip if this is a decent-sized blob
				if (width > DECENT_SIZE)
					break;
			}
		}
	}

	// Add boxes that are farther apart than MAX_ERROR to unique
	vector<Coord> unique;
	sort(boxes.begin(), boxes.end(), box_sort);
	
	// Our comparison below will start at element 1, so initially
	// add the first box
	if (boxes.size() > 0)
		unique.push_back(boxes[0]);
	
	for (unsigned int i = 1; i < boxes.size(); ++i)
		if (abs(1.0*boxes[i].y - boxes[i-1].y) > MAX_ERROR || abs(1.0*boxes[i].x - boxes[i-1].x) > MAX_ERROR)
			unique.push_back(boxes[i]);

	return unique;
}
