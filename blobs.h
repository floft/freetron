/*
 * Take an image (Pixels) and find all black blobs/objects within it.
 *
 *   const Blobs blobs(img);
 *   for (const CoordPair& b : blobs)
 *     std::cout << b.first << std::endl;
 */

#ifndef H_BLOBS
#define H_BLOBS

#include <map>
#include <vector>

#include "data.h"
#include "pixels.h"
#include "maputils.h"
#include "disjointset.h"

// Remember the first and last times we saw a label so we can search just part
// of the image when updating a label.
struct CoordPair
{
    Coord first;
    Coord last;

    CoordPair() { }
    CoordPair(const Coord& f, const Coord& l)
        :first(f), last(l) { }
};

class Blobs
{
public:
    static const int default_label;
    typedef std::map<int, CoordPair>::size_type size_type;
    typedef MapValueIterator<std::map<int, CoordPair>> const_iterator;

private:
    int w = 0;
    int h = 0;
    DisjointSet<int> set;
    std::map<int, CoordPair> objs;
    std::vector<std::vector<int>> labels;

public:
    Blobs(const Pixels& img);
    int label(const Coord& p) const;
    CoordPair object(int label) const;

    // Allow moving
    Blobs(Blobs&&);
    Blobs& operator=(Blobs&& other);

    // Get all first points that have part of the object in the rectangle
    // around p1 and p2 (with p1 to the left and above p2).
    std::vector<Coord> in(const Coord& p1, const Coord& p2) const;

    // Get all the first points of the label within a rectangle around
    // the points p1 and p2. This assumes that p2 is down and to the right
    // of p1.
    std::vector<Coord> startIn(const Coord& p1, const Coord& p2) const;

    // Standard functions
    const_iterator begin() const { return objs.begin(); }
    const_iterator end() const { return objs.end(); }
    size_type size() const { return objs.size(); }

private:
    // Merge object o into object n by changing labels and updating object
    void switchLabel(int old_label, int new_label);
};

#endif
