#include "boxes.h"

// Find all the boxes in the image	TODO: add const
std::vector<Coord> findBoxes(Pixels& img, const Blobs& blobs, BoxData* data)
{
	std::vector<Coord> boxes;

	for (const CoordPair& pair : blobs)
	{
		Box box(&img, blobs, pair.first, data);

		if (box.valid())
		{
			// TODO: what if this isn't quite exact?
			if (boxes.size() == 0)
				data->width = box.width();
			
			boxes.push_back(box.midpoint());

			// We will still scan boxes multiple times, once for each leftmost point
			// on a scanline
			//if (std::find(boxes.begin(), boxes.end(), box.midpoint()) == boxes.end())
		}
	}

	return boxes;
}
