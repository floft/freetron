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
	topleft     = edge(point,       Direction::TL);
	topright    = edge(topleft,     Direction::TR);
	bottomright = edge(topright,    Direction::BR);
	bottomleft  = edge(bottomright, Direction::BL);

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

	if (Square(*img, 415, 2600, 20).in(mp))
	{
		static int blah = 0;
		++blah;

		//if (blah > 110)
		if (blah < 10)
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

// Walk an edge in a specified direction
Coord Box::edge(const Coord& point, Direction dir) const
{
	Coord position    = point;
	Coord last_first  = point;
	Coord last_second = point;
	Coord last_third  = point;
	Coord last_fourth = point;
	
	while (true)
	{
		// The preferences for which direction to move
		Coord first, second, third, fourth;

		switch (dir)
		{
			// Movement precedence
			// 1 2 3
			// 0 0 4
			// 0 0 0
			case Direction::TL:
				first  = Coord(position.x-1, position.y-1);
				second = Coord(position.x,   position.y-1);
				third  = Coord(position.x+1, position.y-1);
				fourth = Coord(position.x+1, position.y);
				break;

			// 0 0 1
			// 0 0 2
			// 0 4 3
			case Direction::TR:
				first  = Coord(position.x+1, position.y-1);
				second = Coord(position.x+1, position.y);
				third  = Coord(position.x+1, position.y+1);
				fourth = Coord(position.x,   position.y+1);
				break;

			// 0 0 0
			// 4 0 0
			// 3 2 1
			case Direction::BR:
				first  = Coord(position.x+1, position.y+1);
				second = Coord(position.x,   position.y+1);
				third  = Coord(position.x-1, position.y+1);
				fourth = Coord(position.x-1, position.y);
				break;
			
			// 3 4 0
			// 2 0 0
			// 1 0 0
			case Direction::BL:
				first  = Coord(position.x-1, position.y+1);
				second = Coord(position.x-1, position.y);
				third  = Coord(position.x-1, position.y-1);
				fourth = Coord(position.x,   position.y-1);
				break;

			// Return because breaking would only break out of the switch
			default:
				return point;
		}

		// See if we can move in a direction, save that as the last time we used that preference
		if (img->black(first))
		{
			position   = first;
			last_first = first;
		}
		else if (img->black(second))
		{
			position    = second;
			last_second = second;
		}
		else if (img->black(third))
		{
			position   = third;
			last_third = third;
		}
		else if (img->black(fourth))
		{
			position    = fourth;
			last_fourth = fourth;
		}
		else
		{
			break;
		}

		// If we've gone more than the digonal, we're not in a box
		if (data->diag != 0 && distance(point, position) > data->diag+MAX_ERROR)
			break;
	}

	// Determine which corner we wish to be close to
	Coord dist_base;
	static const Coord tl(0,		0);
	static const Coord tr(img->width()-1,	0);
	static const Coord br(img->width()-1,	img->height()-1);
	static const Coord bl(0, 		img->height()-1);

	switch (dir)
	{
		case Direction::TL: dist_base = tl; break;
		case Direction::TR: dist_base = tr; break;
		case Direction::BR: dist_base = br; break;
		case Direction::BL: dist_base = bl; break;
	}

	// Create a map of distances from last points of different precedence,
	// Pick the closest one
	const std::map<double, Coord> dist = {
		{ distance(dist_base, last_fourth), last_fourth },
		{ distance(dist_base, last_third),  last_third  },
		{ distance(dist_base, last_second), last_second },
		{ distance(dist_base, last_first),  last_first  }
	};
	
	if (Square(*img, 415, 2600, 20).in(last_first) && dir == Direction::TL)
	{
		std::map<double, Coord>::const_iterator iter;

		for (iter = dist.begin(); iter != dist.end(); ++iter)
		{
			std::cout << iter->first << ": " << iter->second << std::endl;
		}

		std::cout << std::endl;
	}

	return mapMinValue<Coord, std::pair<double, Coord>>(dist.begin(), dist.end());
}
