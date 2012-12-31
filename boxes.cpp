#include "boxes.h"

// Find all the boxes in the image	TODO: add const
std::vector<Coord> findBoxes(Pixels& img, BoxData* data)
{
	std::vector<Coord> boxes;

	for (int y = 0; y < img.height(); ++y)
	{
		for (int x = 0; x < img.width(); ++x)
		{
			const Coord point(x, y);

			// Surrounding points that we've run through already. We can only use these
			// four since otherwise we'd skip all black points.
			const Coord left(x-1, y);
			const Coord upleft(x-1, y-1);
			const Coord up(x, y-1);
			const Coord upright(x+1, y-1);

			// If this point is black and we're not already in a black object. This will help
			// decrease the number of times we scan a box multiple times.
			if (img.black(point)  &&
			   !img.black(left)   &&
			   !img.black(upleft) &&
			   !img.black(up)     &&
			   !img.black(upright))
			{
				Box box(&img, point, data);

				if (box.valid())
				{
					// TODO: what if this isn't quite exact?
					if (boxes.size() == 0)
						data->width = box.width();

					// We will still scan boxes multiple times, once for each leftmost point
					// on a scanline
					if (std::find(boxes.begin(), boxes.end(), box.midpoint()) == boxes.end())
						boxes.push_back(box.midpoint());

					// Skip the rest of this box
					//x += box.width();
				}
			}
		}
	}

	return boxes;
}
