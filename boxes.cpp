#include "boxes.h"

// Find all the boxes in the image
vector< vector<Coord> > findBoxes(Pixels& img,
	const unsigned int& max_x, const unsigned int& max_y,
	Image& image)
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
				// of the possible box is almost entirely black
				if (diagonal <= DIAGONAL+MAX_ERROR && diagonal >= DIAGONAL-MAX_ERROR &&
					averageColor(img, midpoint.x, midpoint.y, BOX_HEIGHT/2, max_x, max_y) > MIN_BLACK)
				{
					vector<Coord> v = { left, right	};
					boxes.push_back(v);
				}

				if (x == 19 && y == 642)
				{
					cout << (averageColor(img, midpoint.x, midpoint.y, BOX_HEIGHT/2, max_x, max_y)) << endl;
					/*image.fillColor("blue");
					image.draw(DrawableRectangle(midpoint.x, midpoint.y, midpoint.x+2, midpoint.y+2));
					image.fillColor("");
					image.strokeColor("blue");
					image.draw(DrawableCircle(midpoint.x, midpoint.y, midpoint.x+BOX_HEIGHT/2, midpoint.y+BOX_HEIGHT/2));
					image.strokeColor("");*/
				}
				
				image.fillColor("red");
				image.draw(DrawableRectangle(left.x, left.y, left.x+2, left.y+2));
				image.fillColor("pink");
				image.draw(DrawableRectangle(right.x, right.y, right.x+2, right.y+2));
				
				// We only care about the left-most black blob, skip if this is a decent-sized blob
				if (diagonal > DECENT_SIZE)
					break;
			}
		}
	}

	return boxes;
}
