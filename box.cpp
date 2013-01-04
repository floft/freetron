#include "box.h"

// The priority for which pixels to go to, search forward if no black yet
// or backward for white if first is black (see Box::findEdge)
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

// Find box properties (corners, width, height, aspect ratio, mid point, etc.)
Box::Box(Pixels& img, const Blobs& blobs, const Coord& point, BoxData& data)
	:img(img), blobs(blobs), data(data)
{
	// Current label so we only walk around this object
	const int label = blobs.label(point);

	if (label == Blobs::default_label)
	{
		std::cerr << blobs.size() << " " << label << " " << point << " " << img.width() << " " << img.height() << std::endl;
		throw std::runtime_error("box can't be created from default label");
	}
	
	// Keep track of previous points. If we go to one again, give up. Otherwise
	// we'll get into an infinite loop.
	std::vector<Coord> path;

	// Remember the last few movements. Do half so that when finding the change in
	// direction, we'll mark get the right pixel.
	Forget<int> previous(PIXEL_RECALL/2, -1);

	// Start one pixel above this first point (we wouldn't have been given this
	// point if the pixel above it was black)
	Coord position(point.x, point.y-1);
	
	// End PIXEL_RECALL pixels in
	Coord end;

	// Direction we're walking
	Direction dir = Direction::TR;
	Direction dir_prev;

	// A fail-safe
	int iterations = 0;

	bool test = false;
	//if (Square(img, 150, 552, 5).in(point))
	//	test = true;
	
	// Walk the edge of the box keeping track of the corners to use later. Corners
	// are where we turn from TR to BR, BR to BL, etc.
	// 
	// Note: "break" gets out of the loop, "return" gives up since this isn't a box
	while (iterations < MAX_ITERATIONS)
	{
		++iterations;

		if (test)
			std::cout << position << " " << dir << std::endl;

		// Find next pixel to go to
		int index = findEdge(position, label, path);

		// All black, should never happen since we already analyzed the image (Blobs)
		if (index == -1)
			throw std::runtime_error("all surrounding pixels are black");

		// Go to new position
		position += matrix[index];
		previous.remember(index);
		
		// Really start after a few pixels
		if (iterations == PIXEL_RECALL)
		{
			end = position;
		}
		else if (iterations > PIXEL_RECALL)
		{
			// We've walked around the entire object now
			if (position == end)
				break;

			// Check if we're going in circles (reversed since most likely we're
			// going back to recent pixels)
			if (std::find(path.rbegin(), path.rend(), position) != path.rend())
				return;
		
			path.push_back(position);
		}
		
		// New direction
		dir_prev = dir;
		dir = findDirection(previous, position, label, path);

		if (dir == Direction::Unknown)
			dir = dir_prev;

		// Save this point as a corner
		if (dir_prev == Direction::TL && dir == Direction::TR)
			topleft = position;
		else if (dir_prev == Direction::TR && dir == Direction::BR)
			topright = position;
		else if (dir_prev == Direction::BR && dir == Direction::BL)
			bottomright = position;
		else if (dir_prev == Direction::BL && dir == Direction::TL)
			bottomleft = position;

		// If we've gone more than the diagonal, we're definitely not in a box
		if (data.diag != 0 && distance(point, position) > data.diag+DIAG_ERROR)
			return;
	}

	// Looks like we have a quadrilateral, it might be a box
	w  = distance(topleft, topright);
	h  = distance(topleft, bottomleft);
	mp = Coord((topleft.x + bottomright.x)/2, (topleft.y + bottomright.y)/2);
	ar = (h>0)?1.0*w/h:0;

	// If we haven't returned already, we might be a box
	possibly_valid = true;

	/*img.mark(topleft);
	img.mark(topright);
	img.mark(bottomleft);
	img.mark(bottomright);*/

	if (test) std::cout << "MP: " << mp << std::endl;
}

bool Box::valid()
{
	// If something when wrong while finding corners, nothing below applies
	if (!possibly_valid)
		return false;
	
	// What should the width be approximately given the aspect ratio (width/height)
	const double approx_height = w/ASPECT;
	const int real_diag = std::ceil(std::sqrt(w*w+h*h));

	/*if (Square(img, 165, 558, 5).in(mp))
	{
		std::cout << w << " " << h << " " << approx_height << " " << real_diag << " " << data.diag << std::endl;
		std::cout << (h >= approx_height-HEIGHT_ERROR && h <= approx_height+HEIGHT_ERROR) << " "
			  << (std::abs(distance(topleft, bottomright) - distance(topright, bottomleft)) < DIAG_ERROR) << " "
			  << (real_diag >= MIN_DIAG && real_diag <= MAX_DIAG) << " "
			  << boxColor() << std::endl;
	}*/

	// See if the diag is about the right length, if the width and height are about right,
	// and if a circle in the center of the possible box is almost entirely black.
	if (h >= approx_height-HEIGHT_ERROR && h <= approx_height+HEIGHT_ERROR &&
		std::abs(distance(topleft, bottomright) - distance(topright, bottomleft)) < DIAG_ERROR && // Diagonals same
		std::abs(distance(topleft, bottomleft) - distance(topright, bottomright)) < DIAG_ERROR && // Height same
		std::abs(distance(topleft, topright) - distance(bottomleft, bottomright)) < DIAG_ERROR && // Width same
		std::abs(slopeXY(topleft, bottomleft) - slopeXY(topright, bottomright)) < SLOPE_ERROR_HEIGHT && // Height slope
		std::abs(slopeYX(topleft, topright) - slopeYX(bottomleft, bottomright)) < SLOPE_ERROR_WIDTH  && // Width slope
		real_diag >= MIN_DIAG && real_diag <= MAX_DIAG && // Get rid of 1-5px boxes
		(
			data.diag == 0 ||
			(real_diag >= data.diag-DIAG_ERROR && real_diag <= data.diag+DIAG_ERROR) // Use found valid diagonal
		) &&
		boxColor() > MIN_BLACK)	// A black box
	{
		img.mark(topleft);
		img.mark(topright);
		img.mark(bottomleft);
		img.mark(bottomright);

		// This is a valid box, so use this diagonal to speed up calculations on next box
		// But, nothing worse than incorrect values, better to not use this than
		// stop searching for the corners early. Thus, make sure there's DIAG_COUNT similar
		// boxes before using the diagonal value.
		if (data.diag == 0)
		{
			// Get test diagonals
			if (data.diags.size() < DIAG_COUNT)
			{
				data.diags.push_back(real_diag);
			}
			// See if they're valid, and use it if they are; otherwise, try again
			else
			{
				// TODO: throw out only bad diags instead of starting over?
				if (absurdDiagonal())
					data.diags.clear();
				else
					data.diag = real_diag;
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
	if (data.diags.size() < 2)
		return true;
	
	// TODO: sort the diags?

	for (size_type i = 1; i < data.diags.size(); ++i)
		if (data.diags[i] > data.diags[i-1]+DIAG_ERROR || data.diags[i] < data.diags[i-1]-DIAG_ERROR)
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
	Square bounds(img, mp.x, mp.y, (bottomright.x - topleft.x)/2);

	for (int y = bounds.topLeft().y; y <= bounds.bottomRight().y; ++y)
	{
		for (int x = bounds.topLeft().x; x <= bounds.bottomRight().x; ++x)
		{
			if (y >= lineFunctionY(topleft,    topright,    x) &&
			    y <= lineFunctionY(bottomleft, bottomright, x) &&
			    x >= lineFunctionX(topleft,    bottomleft,  y) &&
			    x <= lineFunctionX(topright,   bottomright, y))
			{
				if (img.black(Coord(x,y)))
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
//
// If we've been to pixel before, go back till we can go some place new.
int Box::findEdge(const Coord& p, int label,
	const std::vector<Coord>& path) const
{
	typedef std::array<Coord, 8>::size_type size_type;
	
	int index = 0;
	int result = -1;
	size_type checked = 0;

	// If we've checked them all, give up
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

	return result;
}

// Count how many times out of the last floor(PIXEL_RECALL/2) and the next
// ceil(PIXEL_RECALL/2) movements (so we actually get the pixel when it's changing
// direction) we have been going up, right, down, or left (up as in matrix indexes
// 0-2, right 2-4, ...). Whichever one has been most common is the general direction.
Direction Box::findDirection(const Forget<int>& f, const Coord& p,
	int label, const std::vector<Coord>& path) const
{
	Coord lookahead = p;
	std::vector<int> movements(std::ceil(1.0*PIXEL_RECALL/2));

	// Look ahead
	for (int& m : movements)
	{
		m = findEdge(lookahead, label, path);

		if (m == -1)
			break;
		else
			lookahead += matrix[m];
	}

	std::map<Direction, int> sums;
	sums[Direction::TL] = 0;
	sums[Direction::TR] = 0;
	sums[Direction::BL] = 0;
	sums[Direction::BR] = 0;

	// Look at previous and next few positions
	std::vector<int> combined = f.dump();
	combined.insert(combined.end(), movements.begin(), movements.end());

	// Note that the corner positions will be counted as two directions
	for (int index : combined)
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
	
	// If there was no max, we don't know. If there's two tied for max,
	// we're changing direction.
	if (sums[max] == 0 || mapCountValue(sums, sums[max]) > 1)
		return Direction::Unknown;
	else
		return max;
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
