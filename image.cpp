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
	unsigned int r2     = r*r; // Maybe this makes it a bit faster

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

// Is this pixel black?
bool isBlack(Pixels& img, const unsigned int& x, const unsigned int& y)
{
	const PixelPacket *pixel = img.getConst(x, y, 1, 1);
	const ColorGray c(*pixel);

	return c.shade() < GRAY_SHADE;
}

// Find mid point
Coordinate midPoint(const Coordinate& p1, const Coordinate&p2)
{
	return Coordinate((p1.x()+p2.x())/2, (p1.y()+p2.y())/2);
}

// Find leftmost coordinate of box (default top if multiple points)
Coordinate leftmost(Pixels& img,
	const unsigned int& x, const unsigned int& y,
	const unsigned int& max_x, const unsigned int& max_y)
{
	bool found = false;
	unsigned int white_count = 0;
	unsigned int final_x = x;
	unsigned int final_y = y;

	// Continue till a leftmost point is found or we are beyond what could be a box
	while (!found && distance(x, y, final_x, final_y) <= DIAGONAL+MAX_ERROR)
	{
		// Go up till $error white pixels, top_y is highest black pixel
		unsigned int top_y = final_y;
		white_count = 0;
		for (unsigned int search_y = final_y; search_y > 0 && white_count <= MAX_ERROR; --search_y)
			if (isBlack(img, final_x, search_y))
				top_y = search_y;
			else
				++white_count;

		// Go down till $error white pixels, bottom_y is lowest black pixel
		unsigned int bottom_y = final_y;
		white_count = 0;
		for (unsigned int search_y = final_y; search_y < max_y && white_count <= MAX_ERROR; ++search_y)
			if (isBlack(img, final_x, search_y))
				bottom_y = search_y;
			else
				++white_count;

		// Go left from average of top and bottom black points
		unsigned int left_x = final_x;
		final_y = (top_y+bottom_y)/2;
		white_count = 0;
		for (unsigned int search_x = final_x; search_x > 0 && white_count <= MAX_ERROR; --search_x)
			if (isBlack(img, search_x, final_y))
				left_x = search_x;
			else
				++white_count;

		//*(img.get(final_x, final_y, 1, 1)) = Color("pink");

		// If we haven't gone left any, we found the leftmost point
		if (left_x == final_x)
		{
			// Go up to verify we always end up at a single point (there may be more
			// than one left point, so default to the top one)
			white_count = 0;
			for (unsigned int search_y = final_y; search_y > 0 && white_count <= MAX_ERROR; --search_y)
				if (isBlack(img, final_x, search_y))
					final_y = search_y;
				else
					++white_count;
			
			// Done here
			found = true;
		}
		// If we have gone left, do all of this again for the next leftmost point
		else
		{
			final_x = left_x;
		}
	}

	return Coordinate(final_x, final_y);
}

// Find rightmost coordinate of box (default bottom if multiple points)
Coordinate rightmost(Pixels& img,
	const unsigned int& x, const unsigned int& y,
	const unsigned int& max_x, const unsigned int& max_y)
{
	bool found = false;
	unsigned int white_count = 0;
	unsigned int final_x = x;
	unsigned int final_y = y;

	// Continue till a rightmost point is found or we are beyond what could be a box
	while (!found && distance(x, y, final_x, final_y) <= DIAGONAL+MAX_ERROR)
	{
		// Go up till $error white pixels, top_y is highest black pixel
		unsigned int top_y = final_y;
		white_count = 0;
		for (unsigned int search_y = final_y; search_y > 0 && white_count <= MAX_ERROR; --search_y)
			if (isBlack(img, final_x, search_y))
				top_y = search_y;
			else
				++white_count;

		// Go down till $error white pixels, bottom_y is lowest black pixel
		unsigned int bottom_y = final_y;
		white_count = 0;
		for (unsigned int search_y = final_y; search_y < max_y && white_count <= MAX_ERROR; ++search_y)
			if (isBlack(img, final_x, search_y))
				bottom_y = search_y;
			else
				++white_count;

		// Go right from average of top and bottom black points
		unsigned int right_x = final_x;
		final_y = (top_y+bottom_y)/2;
		white_count = 0;
		for (unsigned int search_x = final_x; search_x < max_x && white_count <= MAX_ERROR; ++search_x)
			if (isBlack(img, search_x, final_y))
				right_x = search_x;
			else
				++white_count;

		// If we haven't gone right any, we found the rightmost point
		if (right_x == final_x)
		{
			// Go down to verify we always end up at a single point (there may be more
			// than one right point, so default to the bottom one)
			white_count = 0;
			for (unsigned int search_y = final_y; search_y < max_y && white_count <= MAX_ERROR; ++search_y)
				if (isBlack(img, final_x, search_y))
					final_y = search_y;
				else
					++white_count;
			
			// Done here
			found = true;
		}
		// If we have gone left, do all of this again for the next leftmost point
		else
		{
			final_x = right_x;
		}
	}

	return Coordinate(final_x, final_y);
}
