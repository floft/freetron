/*
 * Class to allow pixel access and rotation to an image
 * 
 * Note: Remember to ilInit() before using this
 */

#ifndef H_PIXELS
#define H_PIXELS

#include <mutex> // Lock needed for Pixels.save()
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <IL/il.h>

#include "math.h"
#include "data.h"
#include "options.h"

class Pixels
{
	std::vector<Coord> marks;
	std::vector<std::vector<unsigned char>> p;
	int w;
	int h;
	bool loaded;

	// Lock this so that only one thread can save() at the same time
	static std::mutex write_lock;

public:
	Pixels(); // Useful for placeholder
	Pixels(ILenum type, const char* lump, const int size);

	inline bool valid()  const { return loaded; }
	inline int  width()  const { return w; }
	inline int  height() const { return h; }

	// This doesn't extend the image at all. If rotation and points
	// are determined correctly, it won't rotate out of the image.
	// Note: rad is angle of rotation in radians
	void rotate(double rad, Coord point = Coord(0,0));

	// Default is used if coord doesn't exist (which should never happen)
	// Default to white to assume that this isn't a useful pixel
	bool black(const Coord& c, const bool default_value = false) const;

	// Used for debugging, marks are written to a copy of the image when saved
	void mark(const Coord& m);
	void save(const std::string& filename) const;
};

#endif
