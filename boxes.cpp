#include "boxes.h"

// Sort based on y values of top-left points
bool box_sort(const Coord& v1, const Coord& v2)
{
	return (v1.y < v2.y);
}

// Find all the boxes in the image
std::vector<Coord> findBoxes(Pixels& img, BoxData* data)
{
	typedef std::vector<Coord>::size_type size_type;

	std::vector<Coord> boxes;

	// Find all the boxes searching from down the image going up at a diagonal to the
	// top for each y value. The height+width also scans coming up from the bottom of
	// the image.
	for (int z = 0; z < img.height() + img.width(); ++z)
	{
		for (int x = 0, y = z; x <= z && x < img.width(); ++x, --y)
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
					// If we don't have a valid width yet, use this box
					// TODO: verify this is similar on multiple boxes?
					if (boxes.size() == 0)
						data->width = box.width();

					// Make sure we didn't already have this point
					if (std::find(boxes.begin(), boxes.end(), box.midpoint()) == boxes.end())
						boxes.push_back(box.midpoint());
				}
				
				// We only care about the left-most black blob, skip if this is a decent-sized blob
				if (box.width() > DECENT_SIZE)
					break;
			}
		}
	}

	// Add boxes that are farther apart than MAX_ERROR to unique
	std::vector<Coord> unique;
	std::sort(boxes.begin(), boxes.end(), box_sort);
	
	// Our comparison below will start at element 1, so initially
	// add the first box
	if (boxes.size() > 0)
		unique.push_back(boxes[0]);
	
	for (size_type i = 1; i < boxes.size(); ++i)
		if (std::abs(1.0*boxes[i].y - boxes[i-1].y) > MAX_ERROR || std::abs(1.0*boxes[i].x - boxes[i-1].x) > MAX_ERROR)
			unique.push_back(boxes[i]);

	return unique;
}
