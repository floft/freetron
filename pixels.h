/*
 * Class to allow pixel access and rotation to an image
 * 
 * Note: Remember to ilInit() before using this
 */

#ifndef H_PIXELS
#define H_PIXELS

#include <vector>
#include <IL/il.h>
#include <iostream>	//TODO: REMOVE THIS
#include <stdexcept>

#include "data.h"

class Pixels
{
	vector< vector<float> > p;
	unsigned int w;
	unsigned int h;
	bool loaded;

public:
	Pixels(); // Useful for placeholder
	Pixels(ILenum type, const char* lump, unsigned int size);

	bool valid() const { return loaded; }
	unsigned int width() const { return w; }
	unsigned int height() const { return h; }

	void rotate(double rad, Coord point = Coord(0,0));
	// Default is used if coord doesn't exist (which should never happen)
	// Default to white to assume that this isn't a useful pixel
	float pixel(Coord c, float default_value = 1) const;
	// Used for debugging
	void square(Coord point, unsigned int size);
};

#endif
