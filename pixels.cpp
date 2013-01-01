#include "pixels.h"

// DevIL/OpenIL isn't multithreaded
std::mutex Pixels::lock;

Pixels::Pixels()
	:w(0), h(0), loaded(false)
{
}

// type is either IL_JPG or IL_PNM in this case
Pixels::Pixels(ILenum type, const char* lump, const int size)
	:w(0), h(0), loaded(false)
{
	// Only execute in one thread since DevIL/OpenIL doesn't support multithreading
	std::unique_lock<std::mutex> lck(lock);

	ILuint name;
	ilGenImages(1, &name);
	ilBindImage(name);

	if (ilLoadL(type, lump, static_cast<ILuint>(size)))
	{
		w = ilGetInteger(IL_IMAGE_WIDTH);
		h = ilGetInteger(IL_IMAGE_HEIGHT);
		
		// If the image height or width is larger than int's max, it will appear
		// to be negative. Just don't use extremely large (many gigapixel) images.
		if (w < 0 || h < 0)
			throw std::runtime_error("use a smaller image, can't store dimensions in int");

		// 3 because IL_RGB
		const int total = w*h*3;
		unsigned char* data = new unsigned char[total];

		ilCopyPixels(0, 0, 0, w, h, 1, IL_RGB, IL_UNSIGNED_BYTE, data);
		
		// Move data into a nicer format
		int x = 0;
		int y = 0;
		p = std::vector<std::vector<unsigned char>>(h, std::vector<unsigned char>(w));

		// Start at third
		for (int i = 2; i < total; i+=3)
		{
			// Average for grayscale
			p[y][x] = smartFloor((1.0*data[i-2]+data[i-1]+data[i])/3);
			
			// Increase y every time we get to end of row
			if (x+1 == w)
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
		throw std::runtime_error("could not read image");
	}
	
	ilDeleteImages(1, &name);
}

bool Pixels::black(const Coord& c, const bool default_value) const
{
	if (c.x >= w || c.y >= h ||
	    c.x < 0 || c.y < 0)
		return default_value;
	
	return p[c.y][c.x] < GRAY_SHADE;
}

void Pixels::mark(const Coord& c)
{
	if (c.x > 0 && c.y > 0 &&
	    c.x < w && c.y < h)
		marks.push_back(c);
}

void Pixels::save(const std::string& filename) const
{
	std::vector<std::vector<unsigned char>> copy = p;

	// Draw the marks on a copy of the image
	for (const Coord& c : marks)
	{
		if (MARK_SIZE > 1)
		{
			// Left
			for (int i = c.x; i > c.x-MARK_SIZE && i > 0; --i)
				copy[c.y][i] = MARK_COLOR;
			// Right
			for (int i = c.x; i < c.x+MARK_SIZE && i < w; ++i)
				copy[c.y][i] = MARK_COLOR;
			// Up
			for (int i = c.y; i > c.y-MARK_SIZE && i > 0; --i)
				copy[i][c.x] = MARK_COLOR;
			// Down
			for (int i = c.y; i < c.y+MARK_SIZE && i < h; ++i)
				copy[i][c.x] = MARK_COLOR;
		}
		else
		{
			copy[c.y][c.x] = MARK_COLOR;
		}
	}

	// One thread again
	std::unique_lock<std::mutex> lck(lock);

	// Convert this back to a real black and white image
	ILuint name;
	ilGenImages(1, &name);
	ilBindImage(name);
	ilEnable(IL_FILE_OVERWRITE);
	
	// 3 because IL_RGB
	const int total = w*h*3;
	unsigned char* data = new unsigned char[total];

	// Position in data
	int pos = 0;

	// For some reason the image is flipped vertically when loading it with
	// ilTexImage, so start at the bottom and go up.
	for (int y = h-1; y >= 0; --y)
	{
		for (int x = 0; x < w; ++x)
		{
			// Black or white for RGB
			const unsigned char val = copy[y][x];
			data[pos]   = val;
			data[pos+1] = val;
			data[pos+2] = val;

			// For R, G, and B that we just set
			pos+=3;
		}
	}
	
	ilTexImage(w, h, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, data);
	
	if (!ilSaveImage(filename.c_str()) || ilGetError() == IL_INVALID_PARAM)
		throw std::runtime_error("could not save image");
	
	ilDeleteImages(1, &name);
}

// In the future, it may be a good idea to implement something like the "Rotation by
// Area Mapping" talked about on http://www.leptonica.com/rotation.html
void Pixels::rotate(double rad, const Coord& point)
{
	// Right size, default to white (255 or 1111 1111)
	std::vector<std::vector<unsigned char>> copy(h, std::vector<unsigned char>(w, 0xff));

	// -rad because we're calculating the rotation to get from the new rotated
	// image to the original image. We're walking the new image instead of the
	// original so as to not get blank spots from rounding.
	const double sin_rad = std::sin(-rad);
	const double cos_rad = std::cos(-rad);

	// Rotate image
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			// "Translate" it, and then add back in the point's x and y
			const int trans_x = x - point.x;
			const int trans_y = y - point.y;

			// Find where to copy from
			const int old_x = smartFloor(trans_x*cos_rad + trans_y*sin_rad) + point.x;
			const int old_y = smartFloor(trans_y*cos_rad - trans_x*sin_rad) + point.y;

			// Get rid of invalid points
			if (old_x > 0 && old_y > 0 &&
			    old_x < w && old_y < h)
				copy[y][x] = p[old_y][old_x];
		}
	}

	p = copy;

	// Rotate marks as well
	marks = rotateVector(marks, point, rad);
}

// Rotate all points in a vector (more or less the same as rotating the image)
std::vector<Coord> Pixels::rotateVector(std::vector<Coord> v, const Coord& point, double rad) const
{
	const double sin_rad = std::sin(rad);
	const double cos_rad = std::cos(rad);

	for (Coord& m : v)
	{
		// Translate to origin
		const int trans_x = m.x - point.x;
		const int trans_y = m.y - point.y;

		// Rotate + translate back
		const int new_x = smartFloor(trans_x*cos_rad + trans_y*sin_rad) + point.x;
		const int new_y = smartFloor(trans_y*cos_rad - trans_x*sin_rad) + point.y;

		if (new_x > 0 && new_y > 0 &&
		    new_x < w && new_y < h)
			m = Coord(new_x, new_y);
	}

	return v;
}
