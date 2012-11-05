#include "image.h"

// Average color of all pixels within radius r of (x,y)
// 0 = complete white, 1 = complete black
double averageColor(Pixels& img,   const unsigned int& x,
	const unsigned int& y,     const unsigned int& r,
	const unsigned int& max_x, const unsigned int& max_y)
{
	// Find square around circle of radius r centered at (x,y)
	unsigned int x1 = min((x<r)?0:x-r, max_x);
	unsigned int y1 = min((y<r)?0:y-r, max_y);
	unsigned int x2 = min(x+r, max_x);
	unsigned int y2 = min(y+r, max_y);

	unsigned int mid_x = (x1+x2)/2;
	unsigned int mid_y = (y1+y2)/2;

	const PixelPacket *pixels = img.getConst(x1, y1, x2-x1, y2-y1);
	unsigned int black  = 0;
	unsigned int total  = 0;
	unsigned int r2     = r*r; // Maybe this makes it faster

	for (unsigned int a = y1; a < y2; ++a)
	{
		for (unsigned int b = x1; b < x2; ++b)
		{
			if (pow(abs(b-mid_x),2) + pow(abs(a-mid_y),2) <= r2)
			{
				++total;
				const ColorMono c(*pixels);

				if (c.mono() == false)
					++black;
			}

			++pixels;
		}
	}

	if (total > 0)
		return 1.0*black/total;
	else
		return 0;
}
