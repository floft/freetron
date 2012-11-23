#include "box.h"

// Is this pixel black?
// Note: If changing this, also change if statement in averageColor
bool isBlack(Pixels& img, const unsigned int& x, const unsigned int& y)
{
	const PixelPacket *pixel = img.getConst(x, y, 1, 1);
	const ColorGray c(*pixel);

	return c.shade() < GRAY_SHADE;
}

// Average color of all pixels within radius r of (x,y)
// 0 = complete white, 1 = complete black
double averageColor(Pixels& img,
	const unsigned int& x, const unsigned int& y,
	const unsigned int& r,
	const unsigned int& max_x, const unsigned int& max_y)
{
	// Find square around circle of radius r centered at (x,y)
	const unsigned int x1 = min((x<r)?0:x-r, max_x);
	const unsigned int y1 = min((y<r)?0:y-r, max_y);
	const unsigned int x2 = min(x+r, max_x);
	const unsigned int y2 = min(y+r, max_y);

	const unsigned int mid_x = (x1+x2)/2;
	const unsigned int mid_y = (y1+y2)/2;

	const PixelPacket *pixels = img.getConst(x1, y1, x2-x1, y2-y1);
	const unsigned int r2 = r*r; // Maybe this makes it a bit faster

	unsigned int black  = 0;
	unsigned int total  = 0;

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

// Only useful when copying to this, since otherwise you can't do anything
// with this object
Box::Box()
	:max_x(0), max_y(0), w(0), h(0), ar(0), img(0), mp(0,0), diag(0)
{
}

// TODO: we can't use a value of 0 for diagonal... we're using it before we calculate it
Box::Box(Pixels& pixels, const Coord& point,
	const unsigned int& maxX, const unsigned int& maxY)
	:max_x(maxX), max_y(maxY), w(0), h(0), ar(0), img(&pixels), mp(0,0), diag(0)
{
	const Coord left   = leftmost(point);
	const Coord right  = rightmost(point);
	const Coord top    = topmost(point);
	const Coord bottom = bottommost(point);

	left.display(*img, Color("blue"));
	right.display(*img, Color("blue"));
	top.display(*img, Color("blue"));
	bottom.display(*img, Color("blue"));

	mp = Coord((left.x + right.x)/2, (top.y + bottom.y)/2);
	h  = bottom.y - top.y;
	w  = right.x  - left.x;
	ar = (h>0)?1.0*w/h:0;

	// Find correct diag for a box this tall
	const double approx_width = ASPECT*h;
	diag = ceil(sqrt(pow(approx_width,2)+pow(h,2)));

	cout << mp << " " << approx_width << " " << diag << endl;
}

bool Box::valid() const
{
	// Verify we actually have a reference to something
	if (!img)
		return false;
	
	// What should the width be approximately given the aspect ratio (width/height)
	const double approx_width = ASPECT*h;

	// See if the diag is about the right length, if the width and height are about right,
	// and if a circle in the center of the possible box is almost entirely black.
	if (w >= approx_width-MAX_ERROR && w <= approx_width+MAX_ERROR &&
		averageColor(*img, mp.x, mp.y, h/2, max_x, max_y) > MIN_BLACK)
		return true;

	return false;	
}

// Find mid point
Coord Box::midPoint(const Coord& p1, const Coord& p2) const
{
	return Coord((p1.x+p2.x)/2, (p1.y+p2.y)/2);
}

// Go up till $error pixels, return new y
unsigned int Box::goUp(const Coord& p, const Coord& orig) const
{
	unsigned int new_y = p.y;
	unsigned int white_count = 0;

	// Go until $error white pixels, hit top of image, or diag
	// is greater than possible for a box.
	for (unsigned int search_y = p.y; search_y > 0 && white_count <= MAX_ERROR &&
		distance(p.x, search_y, orig.x, orig.y) <= diag+MAX_ERROR; --search_y)
	{
		if (isBlack(*img, p.x, search_y))
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

unsigned int Box::goLeft(const Coord& p, const Coord& orig) const
{
	unsigned int new_x = p.x;
	unsigned int white_count = 0;

	for (unsigned int search_x = p.x; search_x > 0 && white_count <= MAX_ERROR &&
		distance(search_x, p.y, orig.x, orig.y) <= diag+MAX_ERROR; --search_x)
	{
		if (isBlack(*img, search_x, p.y))
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

unsigned int Box::goDown(const Coord& p, const Coord& orig) const
{
	unsigned int new_y = p.y;
	unsigned int white_count = 0;

	for (unsigned int search_y = p.y; search_y < max_y && white_count <= MAX_ERROR &&
		distance(p.x, search_y, orig.x, orig.y) <= diag+MAX_ERROR; ++search_y)
	{
		if (isBlack(*img, p.x, search_y))
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

unsigned int Box::goRight(const Coord& p, const Coord& orig) const
{
	unsigned int new_x = p.x;
	unsigned int white_count = 0;

	for (unsigned int search_x = p.x; search_x < max_x && white_count <= MAX_ERROR &&
		distance(search_x, p.y, orig.x, orig.y) <= diag+MAX_ERROR; ++search_x)
	{
		if (isBlack(*img, search_x, p.y))
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
Coord Box::leftmost(const Coord& point) const
{
	Coord left = point;

	// Continue till a leftmost point is found or we are beyond what could be a box
	while (distance(point, left) <= diag+MAX_ERROR)
	{
		// Go up and down till white, find midpoint
		left.y = (goUp(left, point) + goDown(left, point))/2;

		// Go left from average of top and bottom black points
		unsigned int left_x = goLeft(left, point);

		// If we haven't gone left any, we found the leftmost point
		if (left_x == left.x)
			break;

		// If we have gone left, do all of this again for the next leftmost point
		left.x = left_x;
	}

	return left;
}

// Find the topmost coordinate of a box, default left
Coord Box::topmost(const Coord& point) const
{
	Coord top = point;

	// Go left and right, find midpoint. Go up. If we can't move, we found it.
	while (distance(point, top) <= diag+MAX_ERROR)
	{
		top.x = (goLeft(top, point) + goRight(top, point))/2;

		unsigned int top_y = goUp(top, point);

		if (top_y == top.y)
			break;

		top.y = top_y;
	}

	return top;
}

// Find the rightmost coordinate of a box, default down
Coord Box::rightmost(const Coord& point) const
{
	Coord right = point;

	// Continue till a rightmost point is found or we are beyond what could be a box
	while (distance(point, right) <= diag+MAX_ERROR)
	{
		// Go up and down till white, find midpoint
		right.y = (goUp(right, point) + goDown(right, point))/2;

		// Go right from average of top and bottom black points
		unsigned int right_x = goRight(right, point);

		// If we haven't gone right any, we found the rightmost point
		if (right_x == right.x)
			break;

		// If we have gone left, do all of this again for the next leftmost point
		right.x = right_x;
	}

	return right;
}

// Find the bottommost coordinate of a box, default right
Coord Box::bottommost(const Coord& point) const
{
	Coord bottom = point;

	// Go left and right, find midpoint. Go down. If we can't move, we found it.
	while (distance(point, bottom) <= diag+MAX_ERROR)
	{
		bottom.x = (goLeft(bottom, point) + goRight(bottom, point))/2;

		unsigned int bottom_y = goDown(bottom, point);

		if (bottom_y == bottom.y)
			break;

		bottom.y = bottom_y;
	}

	return bottom;
}
