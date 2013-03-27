#include "boxes.h"

// Find all the boxes in the image  TODO: add const
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
            }
        }
    }

    // If we're missing some, estimate their location and see if there's an
    // object there.
    if (boxes.size() < TOTAL_BOXES)
    {
        // TODO
    }

    // If we have too many, throw the ones not in line or on the bottom line out
    if (boxes.size() > TOTAL_BOXES)
    {
        // TODO
    }

    return boxes;
}
