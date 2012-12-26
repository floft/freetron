#include "box.h"

// Find square around point
Square::Square(const Pixels& img, const int x, const int y, const int r)
{
	const int x1 = std::min((x<=r)?0:x-r, img.width()-1);
	const int y1 = std::min((y<=r)?0:y-r, img.height()-1);
	topleft      = Coord(x1, y1);

	const int x2 = std::min(x+r, img.width()-1);
	const int y2 = std::min(y+r, img.height()-1);
	bottomright  = Coord(x2, y2);

	const int mid_x = (x1+x2)/2;
	const int mid_y = (y1+y2)/2;
	midpoint        = Coord(mid_x, mid_y);
}

// Useful for debugging
bool Square::in(const Coord& c) const
{
	return (c.x >= topleft.x && c.x <= bottomright.x &&
	        c.y >= topleft.y && c.y <= bottomright.y);
}

// Average color of all pixels within radius r of (x,y)
// 0 = complete white, 1 = complete black
double averageColor(const Pixels& img,
	const int x, const int y,
	const int r)
{
	// Find square around circle of radius r centered at (x,y)
	Square s(img, x, y, r);
	const int x1    = s.topLeft().x;
	const int y1    = s.topLeft().y;
	const int x2    = s.bottomRight().x;
	const int y2    = s.bottomRight().y;
	const int mid_x = s.midPoint().x;
	const int mid_y = s.midPoint().y;

	// Maybe this makes it a bit faster
	const int r2 = r*r;

	int black  = 0;
	int total  = 0;

	for (int search_y = y1; search_y < y2; ++search_y)
	{
		for (int search_x = x1; search_x < x2; ++search_x)
		{
			if (std::pow(std::abs(search_x-mid_x),2) + std::pow(std::abs(search_y-mid_y),2) <= r2)
			{
				if (img.black(Coord(search_x, search_y)))
					++black;

				++total;
			}
		}
	}
	
	if (total > 0)
		return 1.0*black/total;
	else
		return 0;
}

Box::Box(Pixels* pixels, const Coord& point, BoxData* data)
	:img(pixels), data(data)
{
	const Coord dark   = findDark(point);
	const Coord left   = leftmost(dark);
	const Coord right  = rightmost(dark);
	const Coord top    = topmost(dark);
	const Coord bottom = bottommost(dark);

	// Rotated counter-clockwise (top is farther right than bottom)
	if (top.x - bottom.x > MAX_ERROR)
	{
		// Top is top-right, left is top-left
		// Bottom is bottom-left, left is top-left
		topleft = topLeft(left, dark);
		topright = topRight(top, dark);
		bottomleft = bottomLeft(bottom, dark);
		bottomright = bottomRight(right, dark);
	}
	// Rotated clockwise (top is farther left than bottom)
	else if (bottom.x - top.x > MAX_ERROR)
	{
		// Top is top-left, right is top-right
		// Bottom is bottom-right, right is top-right
		topleft  = topLeft(top, dark);
		topright = topRight(right, dark);
		bottomleft = bottomLeft(left, dark);
		bottomright = bottomRight(bottom, dark);
	}
	// Not really rotated
	else
	{
		topleft  = topLeft(left, dark);
		topright = topRight(right, dark);
		bottomleft = bottomLeft(left, dark);
		bottomright = bottomRight(right, dark);
	}

	w  = distance(topleft, topright);
	h  = distance(topleft, bottomleft);
	mp = Coord((topleft.x + bottomright.x)/2, (topleft.y + bottomright.y)/2);
	ar = (h>0)?1.0*w/h:0;
}

bool Box::valid()
{
	// Verify we actually have a reference to something
	if (!img)
		return false;
	
	// What should the width be approximately given the aspect ratio (width/height)
	const double approx_height = w/ASPECT;
	const int real_diag = std::ceil(std::sqrt(w*w+h*h));

	//if (Square(*img, 40, 436, 20).in(mp))
	if (Square(*img, 42, 1101, 20).in(mp))
	{
		static int blah = 0;
		++blah;

		//if (blah > 110)
		if (blah < 2)
		{
			img->mark(topleft);
			img->mark(topright);
			img->mark(bottomleft);
			img->mark(bottomright);

			std::cout << w << " " << h << " " << approx_height << " " << real_diag << " " << data->diag << std::endl;
			std::cout << (h >= approx_height-MAX_ERROR && h <= approx_height+MAX_ERROR) << " "
				  << (real_diag >= MIN_DIAG && real_diag <= MAX_DIAG) << " "
				  << boxColor() << std::endl;
		}
	}

	// See if the diag is about the right length, if the width and height are about right,
	// and if a circle in the center of the possible box is almost entirely black.
	if (h >= approx_height-MAX_ERROR && h <= approx_height+MAX_ERROR &&
		std::abs(distance(topleft, bottomright) - distance(topright, bottomleft)) < MAX_ERROR && // A rectangle
		(
			(data->diag == 0 && real_diag >= MIN_DIAG && real_diag <= MAX_DIAG) ||	 // Get rid of 1-5 px boxes
			(real_diag >= data->diag-MAX_ERROR && real_diag <= data->diag+MAX_ERROR) // Use found valid diagonal
		) &&
		boxColor() > MIN_BLACK)	// A black box
	{
		/*img->mark(topleft);
		img->mark(topright);
		img->mark(bottomleft);
		img->mark(bottomright);*/

		// This is a valid box, so use this diagonal to speed up calculations on next box
		// But, nothing worse than incorrect values, better to not use this than
		// stop searching for the corners early. Thus, make sure there's DIAG_COUNT similar
		// boxes before using the diagonal value.
		if (data->diag == 0)
		{
			// Get test diagonals
			if (data->diags.size() < DIAG_COUNT)
			{
				data->diags.push_back(real_diag);
			}
			// See if they're valid, and use it if they are; otherwise, try again
			else
			{
				if (absurdDiagonal())
					data->diags.clear();
				else
					data->diag = real_diag;
			}
		}

		return true;
	}

	return false;	
}

bool Box::absurdDiagonal() const
{
	typedef std::vector<int>::size_type size_type;

	// Need at least two to have something beyond error of the previous one
	if (data->diags.size() < 2)
		return true;

	for (size_type i = 1; i < data->diags.size(); ++i)
		if (data->diags[i] > data->diags[i-1]+MAX_ERROR || data->diags[i] < data->diags[i-1]-MAX_ERROR)
			return true;
	
	return false;
}

// Find mid point
Coord Box::midPoint(const Coord& p1, const Coord& p2) const
{
	return Coord((p1.x+p2.x)/2, (p1.y+p2.y)/2);
}

// Go up till $error pixels, return new y
int Box::goUp(const Coord& p, const Coord& orig) const
{
	int new_y = p.y;
	int white_count = 0;

	// Go until $error white pixels, hit top of image, or diag
	// is greater than possible for a box.
	for (int search_y = p.y; search_y >= 0 && white_count <= MAX_ERROR &&
		(data->diag == 0 || distance(p.x, search_y, orig.x, orig.y) <= data->diag+MAX_ERROR); --search_y)
	{
		if (img->black(Coord(p.x, search_y)))
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

int Box::goLeft(const Coord& p, const Coord& orig) const
{
	int new_x = p.x;
	int white_count = 0;

	for (int search_x = p.x; search_x >= 0 && white_count <= MAX_ERROR &&
		(data->diag == 0 || distance(search_x, p.y, orig.x, orig.y) <= data->diag+MAX_ERROR); --search_x)
	{
		if (img->black(Coord(search_x, p.y)))
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

int Box::goDown(const Coord& p, const Coord& orig) const
{
	int new_y = p.y;
	int white_count = 0;

	for (int search_y = p.y; search_y < img->height() && white_count <= MAX_ERROR &&
		(data->diag == 0 || distance(p.x, search_y, orig.x, orig.y) <= data->diag+MAX_ERROR); ++search_y)
	{
		if (img->black(Coord(p.x, search_y)))
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

int Box::goRight(const Coord& p, const Coord& orig) const
{
	int new_x = p.x;
	int white_count = 0;

	for (int search_x = p.x; search_x < img->width() && white_count <= MAX_ERROR &&
		(data->diag == 0 || distance(search_x, p.y, orig.x, orig.y) <= data->diag+MAX_ERROR); ++search_x)
	{
		if (img->black(Coord(search_x, p.y)))
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
// Find the leftmost coordinate of a box
Coord Box::leftmost(const Coord& point) const
{
	Coord left = point;

	// Continue till a leftmost point is found or we are beyond what could be a box
	while (data->diag == 0 || distance(point, left) <= data->diag+MAX_ERROR)
	{
		// Go up and down till white, find midpoint
		left.y = (goUp(left, point) + goDown(left, point))/2;

		// Go left from average of top and bottom black points
		int left_x = goLeft(left, point);

		// If we haven't gone left any, we found the leftmost point
		if (left_x == left.x)
			break;

		// If we have gone left, do all of this again for the next leftmost point
		left.x = left_x;
	}

	return left;
}

// Find the topmost coordinate of a box
Coord Box::topmost(const Coord& point) const
{
	Coord top = point;

	// Go left and right, find midpoint. Go up. If we can't move, we found it.
	while (data->diag == 0 || distance(point, top) <= data->diag+MAX_ERROR)
	{
		top.x = (goLeft(top, point) + goRight(top, point))/2;

		int top_y = goUp(top, point);

		if (top_y == top.y)
			break;

		top.y = top_y;
	}

	return top;
}

// Find the rightmost coordinate of a box
Coord Box::rightmost(const Coord& point) const
{
	Coord right = point;

	// Continue till a rightmost point is found or we are beyond what could be a box
	while (data->diag == 0 || distance(point, right) <= data->diag+MAX_ERROR)
	{
		// Go up and down till white, find midpoint
		right.y = (goUp(right, point) + goDown(right, point))/2;

		// Go right from average of top and bottom black points
		int right_x = goRight(right, point);

		// If we haven't gone right any, we found the rightmost point
		if (right_x == right.x)
			break;

		// If we have gone left, do all of this again for the next leftmost point
		right.x = right_x;
	}

	return right;
}

// Find the bottommost coordinate of a box
Coord Box::bottommost(const Coord& point) const
{
	Coord bottom = point;

	// Go left and right, find midpoint. Go down. If we can't move, we found it.
	while (data->diag == 0 || distance(point, bottom) <= data->diag+MAX_ERROR)
	{
		bottom.x = (goLeft(bottom, point) + goRight(bottom, point))/2;

		int bottom_y = goDown(bottom, point);

		if (bottom_y == bottom.y)
			break;

		bottom.y = bottom_y;
	}

	return bottom;
}

// Check points on a square around a box to find a spot more black than around the current point
Coord Box::findDark(const Coord& p) const
{
	Coord point = p;
	Square bounds(*img, p.x, p.y, DARK_RAD);
	double darkest = averageColor(*img, p.x, p.y, DARK_RAD);

	for (int y = bounds.topLeft().y; y <= bounds.bottomRight().y; y+=DARK_RAD)
	{
		for (int x = bounds.topLeft().x; x <= bounds.bottomRight().x; x+=DARK_RAD)
		{
			double current = averageColor(*img, x, y, DARK_RAD);

			// Greater than because 1 = pure black
			if (current > darkest)
			{
				point = Coord(x,y);
				darkest = current;
			}
		}
	}

	return point;
}

// Get average color of pixels within the corners of the box
double Box::boxColor() const
{
	int black = 0;
	int total = 0;
	Square bounds(*img, mp.x, mp.y, (bottomright.x - topleft.x)/2);

	for (int y = bounds.topLeft().y; y <= bounds.bottomRight().y; ++y)
	{
		for (int x = bounds.topLeft().x; x <= bounds.bottomRight().x; ++x)
		{
			if (y >= lineFunctionY(topleft,    topright,    x) &&
			    y <= lineFunctionY(bottomleft, bottomright, x) &&
			    x >= lineFunctionX(topleft,    bottomleft,  y) &&
			    x <= lineFunctionX(topright,   bottomright, y))
			{
				if (img->black(Coord(x,y)))
					++black;

				++total;
			}
		}
	}

	if (total > 0)
		return 1.0*black/total;
	else
		return 0;
}
