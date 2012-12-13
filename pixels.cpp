#include "pixels.h"

Pixels::Pixels()
	:w(0), h(0), loaded(false)
{
}

// type is either IL_JPG or IL_PNM in this case
Pixels::Pixels(ILenum type, const char* lump, const unsigned int size)
	:w(0), h(0), loaded(false)
{
	ILuint name;
	ilGenImages(1, &name);
	ilBindImage(name);

	if (ilLoadL(type, lump, static_cast<ILuint>(size)))
	{
		ILuint width  = ilGetInteger(IL_IMAGE_WIDTH);
		ILuint height = ilGetInteger(IL_IMAGE_HEIGHT);

		// These will come in handy later (duh)
		w = width;
		h = height;
		
		// 3 because IL_RGB
		const unsigned int total = width*height*3;
		unsigned char* data = new unsigned char[total];

		ilCopyPixels(0, 0, 0, width, height, 1, IL_RGB, IL_UNSIGNED_BYTE, data);
		
		// Move data into a nicer format
		unsigned int x = 0;
		unsigned int y = 0;
		p = vector<vector<bool>>(height, vector<bool>(width));

		// Start at third
		for (unsigned int i = 2; i < total; i+=3)
		{
			// Average for grayscale, just save if it's black or not
			p[y][x] = (1.0*data[i-2]+data[i-1]+data[i])/3 < GRAY_SHADE;
			
			// Increase y every time we get to end of row
			if (x+1 == width)
			{
				x=0;
				++y;
			}
			else
			{
				++x;
			}
		}

		loaded = true;
		delete[] data;
	}
	else
	{
		throw runtime_error("could not read image");
	}
	
	ilDeleteImages(1, &name);
}

bool Pixels::black(Coord c, const bool default_value) const
{
	if (c.x > w || c.y > h)
		return default_value;
	
	return p[c.y][c.x];
}

void Pixels::mark(const Coord& c)
{
	marks.push_back(c);
}

void Pixels::save(const string& filename) const
{
	vector<vector<bool>> copy = p;

	// Draw the marks on a copy of the image
	for (const Coord& c : marks)
	{
		// Left
		for (unsigned int i = c.x; i > c.x-MARK_SIZE && i > 0; --i)
			copy[c.y][i] = true;
		// Right
		for (unsigned int i = c.x; i < c.x+MARK_SIZE && i < w; ++i)
			copy[c.y][i] = true;
		// Up
		for (unsigned int i = c.y; i > c.y-MARK_SIZE && i > 0; --i)
			copy[i][c.x] = true;
		// Down
		for (unsigned int i = c.y; i < c.y+MARK_SIZE && i < h; ++i)
			copy[i][c.x] = true;
	}

	// Convert this back to a real black and white image
	ILuint name;
	ilGenImages(1, &name);
	ilBindImage(name);
	ilEnable(IL_FILE_OVERWRITE);
	
	// 3 because IL_RGB
	const unsigned int total = w*h*3;
	unsigned char* data = new unsigned char[total];

	// Position in data
	unsigned int pos = 0;

	// For some reason the image is flipped when loading it with ilTexImage, so
	// start at the bottom and go up.
	for (unsigned int y = h-1; y > 0; --y)
	{
		for (unsigned int x = 0; x < w; ++x)
		{
			// Black or white for RGB
			const unsigned char val = (copy[y][x])?0:255;
			data[pos]   = val;
			data[pos+1] = val;
			data[pos+2] = val;

			// For R, G, and B that we just set
			pos+=3;
		}
	}
	
	ilTexImage(w, h, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, data);
	
	if (!ilSaveImage(filename.c_str()) || ilGetError() == IL_INVALID_PARAM)
		throw runtime_error("could not save image");
	
	ilDeleteImages(1, &name);
}

void Pixels::rotate(double rad, Coord point)
{
	vector<vector<bool>> copy(h, vector<bool>(w));

	const double sin_rad = sin(rad);
	const double cos_rad = cos(rad);

	for (unsigned int y = 0; y < h; ++y)
	{
		for (unsigned int x = 0; x < w; ++x)
		{
			// "Translate" it, and then add back in the point's x and y
			const int trans_x = x - point.x;
			const int trans_y = y - point.y;
			const unsigned int new_x = trans_x*cos_rad + trans_y*sin_rad + point.x;
			const unsigned int new_y = trans_y*cos_rad - trans_x*sin_rad + point.y;

			// Get rid of invalid points
			if (new_y < h && new_x < w)
				copy[new_y][new_x] = p[y][x];
		}
	}

	p = copy;
}
