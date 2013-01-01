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
	if (!img)
		throw std::runtime_error("img passed to Box was null");

	// We will want exactly four corners
	int corners = 0;

	// Start one pixel above this first point (we wouldn't have been given this
	// point if the pixel above it was black)
	const Coord orig(point.x, point.y-1);
	
	// We'll create a new point that we'll modify since we need to see when we
	// get back to our original location.
	Coord position = orig;

	// We start off at a point somewhere on the top edge
	Direction dir = Direction::TR;
	
	// Walk edge of box
	while (true)
	{
		//img->mark(position);

		// Possible movements
		const Coord zero  = matrix(position, 0, dir);
		const Coord one   = matrix(position, 1, dir);
		const Coord two   = matrix(position, 2, dir);
		const Coord three = matrix(position, 3, dir);
		const Coord four  = matrix(position, 4, dir);

		if (img->black(two))
		{
			if (img->black(four))
			{
				break;
			}
			else
			{
				position = four;
			}
		}
		else if (img->black(one))
		{
			position = two;
		}
		else if (img->black(zero))
		{
			position = one;
		}
		else if (img->black(three))
		{
			position = zero;
		}
		else
		{
			// We have turned the corner
			++corners;
			
			// Doesn't matter at this point
			if (corners > 4)
				break;

			// Save this point
			switch (dir)
			{
				case Direction::TL: topleft     = position; break;
				case Direction::TR: topright    = position; break;
				case Direction::BR: bottomright = position; break;
				case Direction::BL: bottomleft  = position; break;
				default: throw std::runtime_error("invalid direction");
			}

			// Determine new direction and continue
			dir = findDirection(position);

			// Not a box
			if (dir == Direction::Unknown)
			{
				corners = 0;
				break;
			}

			// If we're back where we started (or we never left it)
			if (position == orig)
				break;

		}

		// If we've gone more than the digonal, we're not in a box
		if (data->diag != 0 && distance(point, position) > data->diag+DIAG_ERROR)
		{
			//std::cout << data->diag << " " << distance(point, position) << std::endl;
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
	
	if (Square(*img, 322, 829, 50).in(point))
	{
		img->mark(topleft);
		img->mark(topright);
		img->mark(bottomleft);
		img->mark(bottomright);
		std::cout << corners << std::endl;
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

// Generate movement preferences depending on the direction we wish to go
Coord Box::matrix(const Coord& p, int index, Direction dir) const
{
	// 2 1 0
	// 4   3
	// 7 6 5
	static const std::array<Coord, 8> matrixTL = {{
		Coord( 1, -1),
		Coord( 0, -1),
		Coord(-1, -1),
		Coord( 1,  0),
		Coord(-1,  0),
		Coord( 1,  1),
		Coord( 0,  1),
		Coord(-1,  1)
	}};

	// 7 4 2
	// 6   1
	// 5 3 0
	static const std::array<Coord, 8> matrixTR = {{
		Coord( 1,  1),
		Coord( 1,  0),
		Coord( 1, -1),
		Coord( 0,  1),
		Coord( 0, -1),
		Coord(-1,  1),
		Coord(-1,  0),
		Coord(-1, -1)
	}};
			
	// 5 6 7
	// 3   4
	// 0 1 2
	static const std::array<Coord, 8> matrixBR = {{
		Coord(-1,  1),
		Coord( 0,  1),
		Coord( 1,  1),
		Coord(-1,  0),
		Coord( 1,  0),
		Coord(-1, -1),
		Coord( 0, -1),
		Coord( 1, -1)
	}};

	// 0 3 5
	// 1   6
	// 2 4 7
	static const std::array<Coord, 8> matrixBL = {{
		Coord(-1, -1),
		Coord(-1,  0),
		Coord(-1,  1),
		Coord( 0, -1),
		Coord( 0,  1),
		Coord( 1, -1),
		Coord( 1,  0),
		Coord( 1,  1)
	}};

	switch (dir)
	{
		case Direction::TL: return p+matrixTL[index];
		case Direction::TR: return p+matrixTR[index];
		case Direction::BR: return p+matrixBR[index];
		case Direction::BL: return p+matrixBL[index];
		default: throw std::runtime_error("genMatrix called without valid direction");
	}
}

Direction Box::findDirection(const Coord& p) const
{
	// Matrix indexes are based on TR (arbitrary choice, any would work)
	Direction tr = Direction::TR;

	if (img->black(matrix(p, 3, tr)))
		return Direction::TR;
	if (img->black(matrix(p, 6, tr)))
		return Direction::BR;
	if (img->black(matrix(p, 4, tr)))
		return Direction::BL;
	if (img->black(matrix(p, 1, tr)))
		return Direction::TL;

	return Direction::Unknown;
}
