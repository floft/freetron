#include <set>
#include <array>
#include <algorithm>

#include "log.h"
#include "blobs.h"

const int Blobs::default_label = 0;

Blobs::Blobs(Blobs&& other)
    : w(other.w), h(other.h), set(std::move(other.set)),
      objs(std::move(other.objs)), labels(std::move(other.labels))
{
}

Blobs& Blobs::operator=(Blobs&& other)
{
    w = other.w;
    h = other.h;
    set = std::move(other.set);
    objs = std::move(other.objs);
    labels = std::move(other.labels);

    return *this;
}

Blobs::Blobs(const Pixels& img)
    : set(default_label)
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

                // Conditions we care about:
                //   One neighboring pixel is black
                //   Multiple black, all the same label
                //   Multiple black, not all the same labels
                //
                // We can determine this by counting the number of black pixels
                // around us and knowing if two are different.
                int count = 0;
                bool different = false;
                std::vector<int> equivalent_labels;

                // We will have at most four since the current pixel takes on
                // the label of one of these 4 surrounding pixels
                equivalent_labels.reserve(4);

                for (const Coord& p : points)
                {
                    if (img.black(p))
                    {
                        if (count == 0)
                        {
                            labels[y][x] = labels[p.y][p.x];
                        }
                        // Detect when we have multiple black pixels around us
                        // with different labels
                        else if (labels[y][x] != labels[p.y][p.x])
                        {
                            different = true;
                            equivalent_labels.push_back(labels[p.y][p.x]);
                        }

                        ++count;
                    }
                }

                // No neighboring pixels black
                if (count == 0)
                {
                    // New label for a potentially new object
                    labels[y][x] = next_label;
                    set.add(next_label);

                    ++next_label;
                }
                // Multiple black with different labels
                else if (count > 1 && different)
                {
                    // Save that these are all equivalent to the current one
                    for (int label : equivalent_labels)
                        set.join(labels[y][x], label);
                }
                // Otherwise: One neighbor black or multiple but all same label,
                // and we already set the current pixel's label
            }
        }
    }

    // Go through again reducing the labeling equivalences
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const Coord point(x, y);
            int currentLabel = labels[y][x];

            if (currentLabel != default_label)
            {
                int repLabel = set.find(currentLabel);

                if (repLabel != set.notfound())
                {
                    labels[y][x] = repLabel;

                    // If not found, add this object
                    if (objs.find(repLabel) == objs.end())
                        objs[repLabel] = CoordPair(point, point);
                    // If it is found, this is the last place we've seen the object
                    else
                        objs[repLabel].last = point;
                }
                else
                {
                    log("couldn't find representative of label");
                }
            }
        }
    }
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
    std::set<int> used_labels;
    std::vector<Coord> subset;

    for (int y = p1.y; y < p2.y; ++y)
    {
        for (int x = p1.x; x < p2.x; ++x)
        {
            if (labels[y][x] != default_label &&
                    used_labels.find(labels[y][x]) == used_labels.end())
            {
                const std::map<int, CoordPair>::const_iterator obj = objs.find(labels[y][x]);

                if (obj != objs.end())
                {
                    subset.push_back(obj->second.first);
                    used_labels.insert(labels[y][x]);
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
