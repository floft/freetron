#include "boxes.h"

// See if the point is in the vector
bool inVector(const vector< vector<Coord> >& boxes, vector<Coord> v)
{
	for (unsigned int i = 0; i < boxes.size(); ++i)
		if (boxes[i][0].x == v[0].x && boxes[i][0].y == v[0].y &&
		    boxes[i][1].x == v[1].x && boxes[i][1].y == v[1].y)
		    return true;
	return false;
}

// Find all the boxes in the image
vector< vector<Coord> > findBoxes(Pixels& img,
	const unsigned int& max_x, const unsigned int& max_y)
{
	vector< vector<Coord> > boxes;

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
				Coord left      = leftmost(img,  x, y, max_x, max_y);
				Coord right     = rightmost(img, x, y, max_x, max_y);
				Coord midpoint  = midPoint(left, right);
				double diagonal = distance(left, right);

				// See if the diagonal is about the right length and if a circle in the center
				// of the possible box is almost entirely black.
				if (diagonal <= DIAGONAL+MAX_ERROR && diagonal >= DIAGONAL-MAX_ERROR &&
					averageColor(img, midpoint.x, midpoint.y, BOX_HEIGHT/2, max_x, max_y) > MIN_BLACK)
				{
					vector<Coord> v = { left, right	};

					// Make sure we didn't already have this point
					if (!inVector(boxes, v))
						boxes.push_back(v);
				}
				
				// We only care about the left-most black blob, skip if this is a decent-sized blob
				if (diagonal > DECENT_SIZE)
					break;
			}
		}
	}

	return boxes;
}
