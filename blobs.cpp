#include "blobs.h"

const int Blobs::default_label = 0;

Blobs::Blobs(const Pixels& img)
{
    w = img.width();
    h = img.height();
    labels = std::vector<std::vector<int>>(h, std::vector<int>(w, default_label));
    
    int next_label = default_label+1;

    // Go through finding black points and making them part of bordering objects if next to
    // one already and otherwise a new object
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const Coord point(x, y);

            if (img.black(point))
            {
                // Yes, on the first pixel of a row and the first row some of these will be out
                // of the image, but img.black() returns false when it's not in the image.
                const std::array<Coord, 4> points = {{
                    Coord(x-1, y),   // Left
                    Coord(x-1, y-1), // Up left
                    Coord(x,   y-1), // Up
                    Coord(x+1, y-1)  // Up right
                }};

                bool found = false;

                // If we're next to an object, we're part of that object
                for (const Coord& p : points)
                {
                    if (img.black(p))
                    {
                        labels[y][x] = labels[p.y][p.x];

                        found = true;
                        break;
                    }
                }
                
                if (found)
                {
                    // This is the latest time we've seen this label
                    objs[labels[y][x]].last = point;

                    // Verify all surrounding points are same object
                    for (const Coord& p : points)
                    {
                        // If they are not, make all of the previous object this object
                        if (img.black(p) && labels[y][x] != labels[p.y][p.x])
                            switchLabel(img, labels[p.y][p.x], labels[y][x]);
                    }
                }
                else
                {
                    // Since we're not next to an object, this is [probably] a new object
                    labels[y][x] = next_label;
                    objs[next_label] = CoordPair(point, point);
                    
                    ++next_label;
                }
            }
        }
    }
}

// Change label o to label n by looping through the y values from the first to
// last coordinates of the object
void Blobs::switchLabel(const Pixels& img, const int old_label, const int new_label)
{
    const Coord& old_first = objs[old_label].first;
    const Coord& old_last  = objs[old_label].last;

    // Go between y values of first and last
    for (int y = old_first.y; y <= old_last.y; ++y)
    {
        // This'll be so fast we don't need to check x coordinates
        for (int x = 0; x < w; ++x)
        {
            // Update label
            if (labels[y][x] == old_label)
                labels[y][x] = new_label;
        }
    }

    // Update object
    Coord& new_first = objs[new_label].first;
    Coord& new_last  = objs[new_label].last;

    if (old_first < new_first)
        new_first = old_first;
    
    if (old_last > new_last)
        new_last = old_last;
    
    // Delete old object
    objs.erase(old_label);
}

int Blobs::label(const Coord& p) const
{
    if (p.x >= 0 && p.x < w &&
        p.y >= 0 && p.y < h)
        return labels[p.y][p.x];
    else
        return default_label;
}

std::vector<Coord> Blobs::in(const Coord& p1, const Coord& p2) const
{
    std::vector<int> used_labels;
    std::vector<Coord> subset;

    for (int y = p1.y; y < p2.y; ++y)
    {
        for (int x = p1.x; x < p2.x; ++x)
        {
            if (labels[y][x] != default_label && std::find(used_labels.begin(),
                used_labels.end(), labels[y][x]) == used_labels.end())
            //if (labels[y][x] != default_label && !std::binary_search(used_labels.begin(),
            //    used_labels.end(), labels[y][x]))
            {
                // This fancy thing because this function is constant and accessing
                // with a map may create it if it doesn't exist... It does, but the
                // compiler doesn't know that.
                const std::map<int, CoordPair>::const_iterator obj = objs.find(labels[y][x]);

                if (obj != objs.end())
                {
                    subset.push_back(obj->second.first);
                    used_labels.push_back(labels[y][x]);
                }
                else
                {
                    log("couldn't find object with label");
                }
            }
        }
    }

    return subset;
}

std::vector<Coord> Blobs::startIn(const Coord& p1, const Coord& p2) const
{
    std::vector<Coord> subset;

    for (const std::pair<int, CoordPair>& p : objs)
    {
        if (p.second.first.y >= p1.y && p.second.first.y <= p2.y &&
            p.second.first.x >= p1.x && p.second.first.x <= p2.x)
            subset.push_back(p.second.first);

        // Break early when we've gone past it
        else if (p.second.first.y > p2.y)
            break;
    }

    return subset;
}

CoordPair Blobs::object(int label) const
{
    const std::map<int, CoordPair>::const_iterator obj = objs.find(label);

    if (obj != objs.end())
        return obj->second;
    else
        return CoordPair();
}
