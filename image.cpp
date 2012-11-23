#include "image.h"

// Is this pixel black?
// Note: If changing this, also change if statement in averageColor
bool isBlack(Pixels& img, const unsigned int& x, const unsigned int& y)
{
	const PixelPacket *pixel = img.getConst(x, y, 1, 1);
	const ColorGray c(*pixel);

	return c.shade() < GRAY_SHADE;
}

// Find mid point
Coord midPoint(const Coord& p1, const Coord& p2)
{
	return Coord((p1.x+p2.x)/2, (p1.y+p2.y)/2);
}

// Go up till $error pixels, return new y
unsigned int goUp(Pixels& img, const Coord& p, const Coord& orig)
{
	unsigned int new_y = p.y;
	unsigned int white_count = 0;

	// Go until $error white pixels, hit top of image, or diagonal
	// is greater than possible for a box.
	for (unsigned int search_y = p.y; search_y > 0 && white_count <= MAX_ERROR &&
		distance(p.x, search_y, orig.x, orig.y) <= DIAGONAL+MAX_ERROR; --search_y)
	{
		if (isBlack(img, p.x, search_y))
		{
			new_y = search_y;
			white_count = 0;
		}
		else
		{
			++white_count;
		}
	}
	
	return new_y;
}

unsigned int goLeft(Pixels& img, const Coord& p, const Coord& orig)
{
	unsigned int new_x = p.x;
	unsigned int white_count = 0;

	for (unsigned int search_x = p.x; search_x > 0 && white_count <= MAX_ERROR &&
		distance(search_x, p.y, orig.x, orig.y) <= DIAGONAL+MAX_ERROR; --search_x)
	{
		if (isBlack(img, search_x, p.y))
		{
			new_x = search_x;
			white_count = 0;
		}
		else
		{
			++white_count;
		}
	}
	
	return new_x;
}

unsigned int goDown(Pixels& img,
	const Coord& p, const Coord& orig,
	const unsigned int& max_y)
{
	unsigned int new_y = p.y;
	unsigned int white_count = 0;

	for (unsigned int search_y = p.y; search_y < max_y && white_count <= MAX_ERROR &&
		distance(p.x, search_y, orig.x, orig.y) <= DIAGONAL+MAX_ERROR; ++search_y)
	{
		if (isBlack(img, p.x, search_y))
		{
			new_y = search_y;
			white_count = 0;
		}
		else
		{
			++white_count;
		}
	}
	
	return new_y;
}

unsigned int goRight(Pixels& img,
	const Coord& p, const Coord& orig,
	const unsigned int& max_x)
{
	unsigned int new_x = p.x;
	unsigned int white_count = 0;

	for (unsigned int search_x = p.x; search_x < max_x && white_count <= MAX_ERROR &&
		distance(search_x, p.y, orig.x, orig.y) <= DIAGONAL+MAX_ERROR; ++search_x)
	{
		if (isBlack(img, search_x, p.y))
		{
			new_x = search_x;
			white_count = 0;
		}
		else
		{
			++white_count;
		}
	}
	
	return new_x;
}
// Find the leftmost coordinate of a box, default up
Coord leftmost(Pixels& img, const Coord& original,
	const unsigned int& max_x, const unsigned int& max_y)
{
	Coord left = original;

	// Continue till a leftmost point is found or we are beyond what could be a box
	while (distance(original, left) <= DIAGONAL+MAX_ERROR)
	{
		// Go up and down till white, find midpoint
		left.y = (goUp(img, left, original) + goDown(img, left, original, max_y))/2;

		// Go left from average of top and bottom black points
		unsigned int left_x = goLeft(img, left, original);

		// If we haven't gone left any, we found the leftmost point
		if (left_x == left.x)
			break;

		// If we have gone left, do all of this again for the next leftmost point
		left.x = left_x;
	}

	// Default up
	//left.y = goUp(img, left, original);

	return left;
}

// Find the topmost coordinate of a box, default left
Coord topmost(Pixels& img, const Coord& original,
	const unsigned int& max_x, const unsigned int& max_y)
{
	Coord top = original;

	// Go left and right, find midpoint. Go up. If we can't move, we found it.
	while (distance(original, top) <= DIAGONAL+MAX_ERROR)
	{
		top.x = (goLeft(img, top, original) + goRight(img, top, original, max_x))/2;

		unsigned int top_y = goUp(img, top, original);

		if (top_y == top.y)
			break;

		top.y = top_y;
	}

	// Default left
	//top.y = goLeft(img, top, original);

	return top;
}

// Find the rightmost coordinate of a box, default down
Coord rightmost(Pixels& img, const Coord& original,
	const unsigned int& max_x, const unsigned int& max_y)
{
	Coord right = original;

	// Continue till a rightmost point is found or we are beyond what could be a box
	while (distance(original, right) <= DIAGONAL+MAX_ERROR)
	{
		// Go up and down till white, find midpoint
		right.y = (goUp(img, right, original) + goDown(img, right, original, max_y))/2;

		// Go right from average of top and bottom black points
		unsigned int right_x = goRight(img, right, original, max_x);

		// If we haven't gone right any, we found the rightmost point
		if (right_x == right.x)
			break;

		// If we have gone left, do all of this again for the next leftmost point
		right.x = right_x;
	}

	// Default down
	//right.y = goDown(img, right, original, max_y);

	return right;
}

// Find the bottommost coordinate of a box, default right
Coord bottommost(Pixels& img, const Coord& original,
	const unsigned int& max_x, const unsigned int& max_y)
{
	Coord bottom = original;

	// Go left and right, find midpoint. Go down. If we can't move, we found it.
	while (distance(original, bottom) <= DIAGONAL+MAX_ERROR)
	{
		bottom.x = (goLeft(img, bottom, original) + goRight(img, bottom, original, max_x))/2;

		unsigned int bottom_y = goDown(img, bottom, original, max_y);

		if (bottom_y == bottom.y)
			break;

		bottom.y = bottom_y;
	}

	// Default right
	//bottom.y = goRight(img, bottom, original, max_x);

	return bottom;
}

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

	for (unsigned int search_y = y1; search_y < y2; ++search_y)
	{
		for (unsigned int search_x = x1; search_x < x2; ++search_x)
		{
			if (pow(abs(search_x-mid_x),2) + pow(abs(search_y-mid_y),2) <= r2)
			{
				const ColorGray c(*pixels);

				if (c.shade() < GRAY_SHADE)
					++black;

				++total;
			}

			++pixels;
		}
	}

	if (total > 0)
		return 1.0*black/total;
	else
		return 0;
}

// Go from a leftmost point to the top left point
Coord topLeft(Pixels& img,
	const unsigned int& x, const unsigned int& y,
	const unsigned int& max_x, const unsigned int& max_y)
{
	const Coord original(x,y);
	Coord left = leftmost(img, original, max_x, max_y);
	Coord top  = topmost(img,  original, max_x, max_y);
	double dist = distance(left, top);

	// Find top and left, if top is within error of height, then it's the top-left point,
	// otherwise it's probably the top-right point, so go with the leftmost point
	if (dist <= BOX_HEIGHT+MAX_ERROR && dist >= BOX_HEIGHT-MAX_ERROR)
		return top;
	else
		return left;
	
	// Go up and right till can't go up anymore
	/*while (distance(original, top) <= DIAGONAL+MAX_ERROR)
	{
		top.y = goUp(img, top, original);

		// Go right till black pixel, end of image, or $error white pixels (wouldn't continue
		// looping if there was a black pixel)
		unsigned int top_x = top.x;

		// Start by moving over one, we already know the current one is black
		for (unsigned int search_x = top.x+1, count = 0; search_x < max_x && count <= MAX_ERROR; ++search_x, ++count)
		{
			if (isBlack(img, search_x, top.y))
			{
				top_x = search_x;
				break;
			}
		}

		if (x == 912 && y == 1895)
		*(img.get(top.x, top.y, 1, 1)) = Color("pink");

		// If we haven't moved, we found the point
		if (top_x == top.x)
			break;
		// Do it again
		else
			top.x = top_x;
	}

	// Finish off by going as far left as possible
	top.x = goLeft(img, top, original);

	return top;*/
}

// Go from a rightmost point to the bottom right point
Coord bottomRight(Pixels& img,
	const unsigned int& x, const unsigned int& y,
	const unsigned int& max_x, const unsigned int& max_y)
{
	// Start out with the "top" point being on the left
	const Coord original(x,y);
	Coord right  = rightmost(img,  original, max_x, max_y);
	Coord bottom = bottommost(img, original, max_x, max_y);
	double dist = distance(right, bottom);

	// Find right and bottom, if bottom is within error of height, then it's the
	// bottom-left point, otherwise it's probably the top-right point, so go with
	// the rightmost point
	if (dist <= BOX_HEIGHT+MAX_ERROR && dist >= BOX_HEIGHT-MAX_ERROR)
		return bottom;
	else
		return right;
	
	// Go up and right till can't go up anymore
	/*while (distance(original, bottom) <= DIAGONAL+MAX_ERROR)
	{
		bottom.y = goDown(img, bottom, original, max_y);

		// Go left till black pixel, end of image, or $error white pixels (wouldn't continue
		// looping if there was a black pixel)
		unsigned int bottom_x = bottom.x;

		// Start by moving over one, we already know the current one is black
		for (unsigned int search_x = bottom.x-1, count = 0; search_x > 0 && count <= MAX_ERROR; --search_x, ++count)
		{
			if (isBlack(img, search_x, bottom.y))
			{
				bottom_x = search_x;
				break;
			}
		}

		// If we haven't moved, we found the point
		if (bottom_x == bottom.x)
			break;
		// Do it again
		else
			bottom.x = bottom_x;
	}

	// Finish off by going as far right as possible
	bottom.x = goRight(img, bottom, original, max_x);

	return bottom;*/
}
