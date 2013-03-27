#include "outline.h"

// The priority for which pixels to go to, search forward if no black yet
// or backward for white if first is black (see Outline::findEdge)
//  0 1 2
//  7   3
//  6 5 4
const std::array<Coord, 8> Outline::matrix = {{
    Coord(-1, -1),
    Coord( 0, -1),
    Coord( 1, -1),
    Coord( 1,  0),
    Coord( 1,  1),
    Coord( 0,  1),
    Coord(-1,  1),
    Coord(-1,  0)
}};

Outline::Outline(const Pixels& img, const Blobs& blobs, const Coord& point,
    const int max_length)
    :img(img), blobs(blobs)
{
    // Current label so we only walk around this object
    label = blobs.label(point);

    if (label == Blobs::default_label)
    {
        log("can't outline object with default label");
        return;
    }
    
    // Start one pixel above this first point (we wouldn't have been given this
    // point if the pixel above it was black)
    Coord position(point.x, point.y-1);
    
    // End where we started
    Coord end = position;

    // A fail-safe
    int iterations = 0;

    // Walk the edge of the box
    // Note: "break" when we're done, "return" gives up since this isn't a box
    while (true)
    {
        // Find next pixel to go to
        EdgePair edge = findEdge(position);

        // Ran out of options (shouldn't happen?)
        if (edge.index == -1)
        {
            log("ran out of options on edge");
            return;
        }

        // Go to new position (note that edge.point will be position unless
        // we had to retrace our steps a ways)
        position = edge.point+matrix[edge.index];
        path.push_back(position);

        // We've walked around the entire object now
        if (position == end)
            break;
        
        // Give up after we reach a certain size of object
        ++iterations;

        if (iterations > max_length)
            return;
    }

    // If we didn't return already, we found the points.
    found = true;
}

// If we've been to pixel before, go back till we can go some place new.
EdgePair Outline::findEdge(const Coord& p) const
{
    typedef std::vector<Coord>::size_type size_type;
    
    int index = -1;

    // Start at this point
    Coord position = p;

    // If that doesn't work, start at the last pixel
    size_type path_index = (path.size()>0)?(path.size()-1):0;

    // Keep going backwards until we find one with another option
    while (true)
    {
        index = findIndex(position);

        // We found an option!
        if (index != -1)
            break;

        // We ran out of options
        if (path_index == 0)
            break;
        
        // Go back one pixel
        position = path[path_index];
        --path_index;
    }

    return EdgePair(position, index);
}

// Loop through the matrix looking for the first black pixel with a white (not
// part of this object) pixel previous to it that we haven't been to. Return -1
// if there's no options (e.g. white pixel in middle of black ring).
int Outline::findIndex(const Coord& p) const
{
    typedef std::array<Coord, 8>::size_type size_type;
    
    int result = -1;

    for (size_type i = 0; i < matrix.size(); ++i)
    {
        // Back one pixel with looping, i.e. 0-1 = 7
        const int back = (i==0)?(matrix.size()-1):(i-1);

        const Coord current  = p+matrix[i];
        const Coord previous = p+matrix[back];

        if (blobs.label(current) == label && blobs.label(previous) != label &&
            std::find(path.rbegin(), path.rend(), previous) == path.rend())
        {
            result = back;
            break;
        }
    }

    return result;
}
