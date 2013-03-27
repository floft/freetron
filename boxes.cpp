#include "boxes.h"

// Find all the boxes in the image
std::vector<Coord> findBoxes(Pixels& img, BoxData& data)
{
    typedef std::vector<Coord>::size_type size_type;

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

    size_type jump = 0;

    // Now find the first box after the jump
    for (size_type i = 1; i < boxes.size(); ++i)
    {
        if (boxes[i].y - boxes[i-1].y > HUGE_JUMP)
        {
            jump = i;
            break;
        }
    }

    // We won't use the first one or two above the jump
    boxes.erase(boxes.begin(), boxes.begin()+jump);

    return boxes;
}
