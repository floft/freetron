#include "box.h"

//  0 1 2
//  7   3
//  6 5 4
const std::array<Coord, 8> Box::matrix = {{
	Coord(-1, -1),
	Coord( 0, -1),
	Coord( 1, -1),
	Coord( 1,  0),
	Coord( 1,  1),
	Coord( 0,  1),
	Coord(-1,  1),
	Coord(-1,  0)
}};

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

Box::Box(Pixels* pixels, const Blobs& blobs, const Coord& point, BoxData* data)
	:img(pixels), data(data)
{
	if (!img)
		throw std::runtime_error("img passed to Box was null");

	// Current label
	const int label = blobs.label(point);

	if (label == Blobs::default_label)
		throw std::runtime_error("box can't be created from default label");
	
	// Keep track of previous movements by the index of the matrix
	Forget<int> previous(PIXEL_RECALL);
	
	// We must go at least CORNER_DIST from last turn to be considered a corner
	int corners = 0;

	// Set this when we get PIXEL_RECALL pixels in, so we known when to stop
	Coord end;

	// The last unknown direction coordinate, which will be used when we eventually
	// determine what direction we are going.
	Coord interest;
	
	// Start one pixel above this first point (we wouldn't have been given this
	// point if the pixel above it was black)
	Coord position(point.x, point.y-1);

	// Direction we're walking, and previous one so we can see if we've changed direction
	Direction dir = Direction::TR;
	Direction dir_prev;

	bool test = false;
	if (Square(*img, 144, 2677, 5).in(point))
		test = true;
	
	// Walk edge of box. As a fail-safe, give up if we've gone more pixels than an
	// outrageously large box.
	while (true)
	{
		img->mark(position);
		std::cout << point << " " << position << std::endl;

		// Find next pixel to go to
		int index = findEdge(position, label, blobs);

		// All black, give up
		if (index == -1)
		{
			if (test) std::cout << "index" << std::endl;
			corners = 0;
			break;
		}

		// Give up if we just turned around. We don't want infinite loops.
		/*if (turnedAround(dir, index))
		{
			std::cout << "Backwards: " << point << std::endl;
			corners = 0;
			break;
		}*/

		/*if (badPixel(point))
		{
			corners = 0;
			break;
		}*/

		if (test)
			std::cout << matrix[index] << ": " << position << " ";

		// Go to new position
		position += matrix[index];

		if (test)
			std::cout << position << std::endl;
		
		// Remember this position for finding corners
		previous.remember(index);

		if (test)
		{
			for (int i : previous)
				std::cout << i << " ";
			std::cout << std::endl;
		}

		// We've walked around the entire object now
		if (position == end)
		{
			if (test) std::cout << "end" << std::endl;
			break;
		}

		// Determine direction if we've gone more than PIXEL_RECALL
		if (previous.count() > PIXEL_RECALL)
		{
			if (end == Coord())
				end = position;

			dir_prev = dir;
			dir = findDirection(previous);

			if (dir != dir_prev)
			{
				// If we know the direction, set this point to the last time
				// we were half way between two directions (Unknown).
				switch (dir)
				{
					case Direction::TL:
						topleft = interest;
						++corners;
						break;
					case Direction::TR:
						topright = interest;
						++corners;
						break;
					case Direction::BR:
						bottomright = interest;
						++corners;
						break;
					case Direction::BL:
						bottomleft = interest;
						++corners;
						break;
					case Direction::Unknown:
						interest = position;
						break;
				}
			}
		}

		// Doesn't matter at this point
		if (corners > 4)
		{
			if (test) std::cout << "corners" << std::endl;
			break;
		}

		// If we've gone more than the diagonal, we're definitely not in a box
		if (data->diag != 0 && distance(point, position) > data->diag+DIAG_ERROR)
		{
			if (test) std::cout << "dist" << std::endl;
			corners = 0;
			break;
		}
	}

	// Looks like we have a quadrilateral, it might be a box
	if (corners == 4)
	{
		w  = distance(topleft, topright);
		h  = distance(topleft, bottomleft);
		mp = Coord((topleft.x + bottomright.x)/2, (topleft.y + bottomright.y)/2);
		ar = (h>0)?1.0*w/h:0;

		possibly_valid = true;
	}
	
	if (test)
	{
		img->mark(topleft);
		img->mark(topright);
		img->mark(bottomleft);
		img->mark(bottomright);
		std::cout << corners << " " << dir << " " <<
			distance(point,position) << " " << data->diag+DIAG_ERROR << std::endl;
	}
}

bool Box::valid()
{
	// Verify we actually have a reference to something
	if (!img)
		return false;
	
	// If we didn't find the four courners, then none of the below checks apply
	if (!possibly_valid)
		return false;
	
	// What should the width be approximately given the aspect ratio (width/height)
	const double approx_height = w/ASPECT;
	const int real_diag = std::ceil(std::sqrt(w*w+h*h));

	if (Square(*img, 300, 1288, 50).in(mp))
	{
		static int blah = 0;
		++blah;

		if (blah < 2)
		{
			img->mark(topleft);
			img->mark(topright);
			img->mark(bottomleft);
			img->mark(bottomright);

			std::cout << topleft << " " << topright << " " << bottomleft << " " << bottomright << std::endl;

			std::cout << w << " " << h << " " << approx_height << " " << real_diag << " " << data->diag << std::endl;
			std::cout << (h >= approx_height-HEIGHT_ERROR && h <= approx_height+HEIGHT_ERROR) << " "
				  << (std::abs(distance(topleft, bottomright) - distance(topright, bottomleft)) < DIAG_ERROR) << " "
				  << (real_diag >= MIN_DIAG && real_diag <= MAX_DIAG) << " "
				  << boxColor() << std::endl;
		}
	}

	// See if the diag is about the right length, if the width and height are about right,
	// and if a circle in the center of the possible box is almost entirely black.
	if (h >= approx_height-HEIGHT_ERROR && h <= approx_height+HEIGHT_ERROR &&
		std::abs(distance(topleft, bottomright) - distance(topright, bottomleft)) < DIAG_ERROR && // A rectangle
		real_diag >= MIN_DIAG && real_diag <= MAX_DIAG && // Get rid of 1-5px boxes
		(
			data->diag == 0 ||
			(real_diag >= data->diag-DIAG_ERROR && real_diag <= data->diag+DIAG_ERROR) // Use found valid diagonal
		) &&
		boxColor() > MIN_BLACK)	// A black box
	{
		img->mark(topleft);
		img->mark(topright);
		img->mark(bottomleft);
		img->mark(bottomright);

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
				// TODO: throw out only bad diags instead of starting over?
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
	
	// TODO: sort the diags?

	for (size_type i = 1; i < data->diags.size(); ++i)
		if (data->diags[i] > data->diags[i-1]+DIAG_ERROR || data->diags[i] < data->diags[i-1]-DIAG_ERROR)
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

// Find the next white pixel to go to by looping through the matrix.
// Look through the matrix starting from index 0 until we hit a black
// pixel. Then, go to the previous pixel if it's not black, otherwise
// continue looking back until we hit a white pixel. If we've gone
// through all of them, return -1.

// Find a white pixel by going around the matrix positions. Then look for
// a black pixel. Go to the white pixel before. Return -1 if there are no
// white pixels.
int Box::findEdge(const Coord& p, int label, const Blobs& blobs) const
{
	typedef std::array<Coord, 8>::size_type size_type;
	
	int index = -1;
	int result = -1;

	// Find first white pixel, if there is one
	for (size_type i = 0; i < matrix.size(); ++i)
	{
		if (blobs.label(p+matrix[i]) != label)
		{
			index = i;
			break;
		}
	}

	// If we found one, now search for a black pixel. Go back one and go there.
	if (index != -1)
	{
		size_type checked = 0;

		while (checked < matrix.size())
		{
			if (blobs.label(p+matrix[index]) == label)
			{
				// Loop around (0-1 = 7)
				int previous = (index==0)?(matrix.size()-1):(index-1);

				if (blobs.label(p+matrix[previous]) != label)
				{
					result = previous;
					break;
				}

				index = previous;
			}
			else
			{
				++index;
			}

			++checked;
		}
	}

	return result;
}

// Count how many times out of the last PIXEL_RECALL movements we have been going
// up, right, down, or left (up as in matrix indexes 0-2, right 2-4, ...). Whichever
// one has been most common of the last PIXEL_RECALL movements is the general direction.
Direction Box::findDirection(const Forget<int>& previous) const
{
	std::map<Direction, int> sums;
	sums[Direction::TL] = 0;
	sums[Direction::TR] = 0;
	sums[Direction::BL] = 0;
	sums[Direction::BR] = 0;

	// Note that the corner positions will be counted as two directions
	for (int index : previous)
	{
		if (index == 0 || index == 1 || index == 2)
			++sums[Direction::TL];
		if (index == 2 || index == 3 || index == 4)
			++sums[Direction::TR];
		if (index == 4 || index == 5 || index == 6)
			++sums[Direction::BR];
		if (index == 0 || index == 6 || index == 7)
			++sums[Direction::BL];
	}

	// Return key (direction) of max value
	Direction max = mapMaxValueKey<Direction, std::pair<Direction, int>>(sums.begin(), sums.end());
	
	//std::cout << sums[Direction::TL] << " " << sums[Direction::TR] << " "
	//	  << sums[Direction::BR] << " " << sums[Direction::BL] << std::endl;

	// If there's two tied for max, we're changing direction.
	if (mapCountValue(sums, sums[max]) > 1)
		return Direction::Unknown;
	else
		return max;
}

// See if our next movement will take us in the opposite direction, thus
// putting us into an infinite loop.
bool Box::turnedAround(Direction dir, int index) const
{
	switch (dir)
	{
		case Direction::TL:
			return index == 5;
		case Direction::TR:
			return index == 7;
		case Direction::BR:
			return index == 1;
		case Direction::BL:
			return index == 3;
		default:
			return false;
	}
}

// Useful for debugging
std::ostream& operator<<(std::ostream& os, const Direction& d)
{
	switch (d)
	{
		case Direction::TL: return os << "TopLeft";
		case Direction::TR: return os << "TopRight";
		case Direction::BL: return os << "BottomLeft";
		case Direction::BR: return os << "BottomRight";
		case Direction::Unknown: return os << "Unknown";
		default: return os << "Unknown?";
	}
}
