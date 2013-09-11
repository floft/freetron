#include "boxes.h"

// Find all the boxes in the image
std::vector<Coord> findBoxes(Pixels& img, const Blobs& blobs, Data& data)
{
    typedef std::vector<Coord>::size_type size_type;

    std::vector<Box> boxes;
    std::vector<Coord> coords;

    for (const CoordPair& pair : blobs)
    {
        // This may be the height, width, or diagonal
        double dist = distance(pair.first, pair.last);

        // Get rid of most the really big or really small objects
        if (dist > MIN_HEIGHT && dist < MAX_DIAG)
        {
            Box box(img, blobs, pair.first);

            if (box.valid())
                boxes.push_back(box);
        }
    }

    bool found = false;
    size_type jump = 0;

    // Now find the first box after the jump
    for (size_type i = 1; i < boxes.size(); ++i)
    {
        // Ignore everything up to this big jump
        if (!found && boxes[i].midpoint().y - boxes[i-1].midpoint().y > HUGE_JUMP)
        {
            jump = i;
            found = true;
        }

        // We'll return the midpoints to use later
        if (found)
            coords.push_back(boxes[i].midpoint());
    }

    // Save this information for the first box after the jump
    if (boxes.size() > 0)
    {
        data.width = boxes[jump].width();
        data.diag  = boxes[jump].diagonal();
    }

    return coords;
}
