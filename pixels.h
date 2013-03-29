/*
 * Class to allow pixel access and rotation to an image
 * 
 * Note: Remember to ilInit() before using this
 */

#ifndef H_PIXELS
#define H_PIXELS

#include <mutex>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <IL/il.h>

#include "math.h"
#include "data.h"
#include "options.h"
#include "histogram.h"

struct Mark
{
    Coord coord;
    int size;

    Mark(Coord c, int s)
        :coord(c), size(s) { }
};

class Pixels
{
    std::vector<Mark> marks;
    std::vector<std::vector<unsigned char>> p;
    int w;
    int h;
    bool loaded;
    std::string fn;
    unsigned char gray_shade;

    // Lock this so that only one thread can read an image or save()
    // OpenIL/DevIL is not multithreaded
    static std::mutex lock;

public:
    Pixels(); // Useful for placeholder
    Pixels(ILenum type, const char* lump, const int size, const std::string& fn = "");

    inline bool valid()  const { return loaded; }
    inline int  width()  const { return w; }
    inline int  height() const { return h; }
    inline const std::string& filename() const { return fn; }

    // This doesn't extend the image at all. If rotation and points
    // are determined correctly, it won't rotate out of the image.
    // Note: rad is angle of rotation in radians
    void rotate(double rad, const Coord& point);

    // Default is used if coord doesn't exist (which should never happen)
    // Default to white to assume that this isn't a useful pixel
    inline bool black(const Coord& c, const bool default_value = false) const;
    
    // When saving, we'll display marks optionally
    void mark(const Coord& m, int size = MARK_SIZE);

    // Mark every pixel on the line between p1 and p2
    void line(const Coord& p1, const Coord& p2);
    
    // Used for debugging, all processing (converting to black-and-white, adding
    // the marks, dimming the image) is done on a copy of the image
    void save(const std::string& filename, const bool show_marks = true,
              const bool dim = true, const bool bw = true) const;

    // Rotate all points in a vector
    void rotateVector(std::vector<Coord>& v, const Coord& point, double rad) const;
};

// Used so frequently and so small, so make this inline
inline bool Pixels::black(const Coord& c, const bool default_value) const
{
    if (c.x >= 0 && c.y >= 0 &&
        c.x < w  && c.y < h)
        return p[c.y][c.x] < gray_shade;

    return default_value;
}

#endif
