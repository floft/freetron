/*
 * Class to allow pixel access and rotation to an image
 * 
 * Note: Remember to ilInit() before using this
 */

#ifndef H_PIXELS
#define H_PIXELS

#include <vector>
#include <string>
#include <stdexcept>
#include <IL/il.h>

#include "data.h"
#include "options.h"

struct Mark
{
	Coord point;
	unsigned int size;

	Mark() :size(MARK_SIZE) { }
	Mark (Coord c) :point(c), size(MARK_SIZE) { }
	Mark (Coord c, const unsigned int s) :point(c), size(s) { }
};

class Pixels
{
	vector<Mark> marks;
	vector<vector<bool>> p;
	unsigned int w;
	unsigned int h;
	bool loaded;

public:
	Pixels(); // Useful for placeholder
	Pixels(ILenum type, const char* lump, const unsigned int size);

	bool valid() const { return loaded; }
	unsigned int width() const { return w; }
	unsigned int height() const { return h; }

	// rad is angle of rotation in radians
	void rotate(double rad, Coord point = Coord(0,0));

	// Default is used if coord doesn't exist (which should never happen)
	// Default to white to assume that this isn't a useful pixel
	bool black(Coord c, const bool default_value = false) const;

	// Used for debugging, marks are written to a copy of this when saved
	void mark(const Mark& m);
	void save(const string& filename) const;
};

#endif
