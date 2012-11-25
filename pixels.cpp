#include "pixels.h"

Pixels::Pixels()
	:w(0), h(0), loaded(false)
{
}

// type is either IL_JPG or IL_PNM in this case
Pixels::Pixels(ILenum type, const char* lump, unsigned int size)
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
		p = vector< vector<float> >(height, vector<float>(width));

		// Start at third
		for (unsigned int i = 2; i < total; i+=3)
		{
			// Average for grayscale
			p[y][x] = (1.0*data[i-2]+data[i-1]+data[i])/3;
			
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

float Pixels::pixel(Coord c, float default_value) const
{
	if (c.x > w || c.y > h)
		return default_value;
	
	return p[c.y][c.x];
}
