#include "boxes.h"

// Find all the boxes in the image	TODO: add const
std::vector<Coord> findBoxes(Pixels& img, BoxData& data)
{
	std::vector<Coord> boxes;

	// Find all objects in image
	const Blobs blobs(img);

	for (const CoordPair& pair : blobs)
	{
		// This may be the height, width, or diagonal
		double dist = distance(pair.first, pair.last);

		// Get rid of most the really big or really small objects
		if (dist > MIN_HEIGHT && dist < MAX_DIAG)
		{
			Box box(img, blobs, pair.first, data);

			if (box.valid())
			{
				// TODO: what if this isn't quite exact?
				if (boxes.size() == 0)
					data.width = box.width();
				
				boxes.push_back(box.midpoint());

				// We will still scan boxes multiple times, once for each leftmost point
				// on a scanline
				//if (std::find(boxes.begin(), boxes.end(), box.midpoint()) == boxes.end())
			}
		}
	}

	return boxes;
}
