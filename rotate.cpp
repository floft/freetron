#include "rotate.h"

// Find top left coordinate of the box containing point (x,y)
Coordinate findTopLeft(Pixels& img,
	const unsigned int& x,     const unsigned int& y,
	const unsigned int& max_x, const unsigned int& max_y)
{
	unsigned int result_x = x;
	unsigned int result_y = y;
	unsigned int missed   = 0;
	unsigned int r = relative_r(max_x, max_y);

	// Search up and then left until hitting color < min_black
	for (unsigned int search_x = x+r, search_y = y; search_x >= 0 && search_x < max_x; --search_x)
	{
		for (search_y = result_y; search_y >= 0 && search_y < max_y; --search_y)
		{
			if (averageColor(img, search_x, search_y, r, max_x, max_y) > min_black)
			{
				result_x = search_x;
				result_y = search_y;
			}
			else break;
		}

		// If we haven't gone up any and are just moving left, break after only a few white
		// pixels instead of going all the way to the wall
		if (missed > max_missed)
			break;
		if (search_y == result_y && averageColor(img, search_x, search_y, r, max_x, max_y) < min_black)
			++missed;
	}

	return Coordinate(result_x, result_y);
}

// Find top right coordinate of the box containing point (x,y)
Coordinate findTopRight(Pixels& img,
	const unsigned int& x,     const unsigned int& y,
	const unsigned int& max_x, const unsigned int& max_y)
{
	unsigned int result_x = x;
	unsigned int result_y = y;
	unsigned int missed   = 0;
	unsigned int r = relative_r(max_x, max_y);

	// Search right then up until hitting color < min_black
	for (unsigned int search_y = y+r, search_x = x; search_y >= 0 && search_y < max_y; --search_y)
	{
		for (search_x = result_x; search_x < max_x; ++search_x)
		{
			if (averageColor(img, search_x, search_y, r, max_x, max_y) > min_black)
			{
				result_x = search_x;
				result_y = search_y;
			}
			else break;
		}

		// If we haven't gone up any and are just moving left, break after only a few white
		// pixels instead of going all the way to the wall
		if (missed > max_missed)
			break;
		if (search_x == result_x && averageColor(img, search_x, search_y, r, max_x, max_y) < min_black)
			++missed;
	}

	return Coordinate(result_x, result_y);
}

// Find boxes on left of image
vector< vector<unsigned int> > findBoxes(Pixels& img,
	const unsigned int& max_x, const unsigned int& max_y,
	unsigned int& box_width, Image& image)
{
	double firstDist = 0;
	vector<Coordinate> boxesLeft;
	vector< vector<unsigned int> > boxes;
	unsigned int r = relative_r(max_x, max_y);
	unsigned int box_size = relative_box(max_x, max_y);

	// Search from left up a y = x line going down the image
	// Max y+x for also scanning the bottom of the image if shifted to the right
	for (unsigned int z = 0; z < max_y + max_x; ++z)
	{
		for (unsigned int x = 0, y = z; x <= z && x < max_x; ++x, --y)
		{
			// This is an imaginary point
			if (y > max_y - 1)
				continue;

			if (averageColor(img, x, y, r, max_x, max_y) > min_black)
			{
				Coordinate topLeft = findTopLeft(img, x, y, max_x, max_y);

				// Add this if it is not already a box and is large enough to be one
				if (find(boxesLeft.begin(), boxesLeft.end(), topLeft) == boxesLeft.end() &&
					averageColor(img, x, y, box_size, max_x, max_y) > min_black)
					boxesLeft.push_back(topLeft);
				else
					break;
				
				// We only care about the left-most black value
				break;
			}
		}
	}

	// Find top right points and delete abnormally sized boxes
	for (unsigned int i = 0; i < boxesLeft.size(); ++i)
	{
		unsigned int x1 = boxesLeft[i].x();
		unsigned int y1 = boxesLeft[i].y();

		Coordinate topRight = findTopRight(img, x1, y1, max_x, max_y);
		
		unsigned int x2 = topRight.x();
		unsigned int y2 = topRight.y();

		double dist = distance(x1, y1, x2, y2);

		// Size of first box is the standard
		if (i == 0)
		{
			firstDist = dist;

			/*cout << dist << endl;
			if (dist < 40)
			{
				image.fillColor("pink");
				image.draw(DrawableRectangle(x1, y1, x1+15, y1+15));
				image.draw(DrawableRectangle(x2, y2, x2+15, y2+15));
			}*/

			vector<unsigned int> v = { x1, y1, x2, y2 };
			boxes.push_back(v);
		}
		// Get rid of all that are way larger or smaller than the first
		else if (dist < (1+max_error)*firstDist && dist > (1-max_error)*firstDist)
		{
			vector<unsigned int> v = { x1, y1, x2, y2 };
			boxes.push_back(v);
		}
	}

	// Save the box width for use when reading data
	box_width = firstDist;

	return boxes;
}

// Find 2 closest values in an array and return percentError
/*vector<double> findClosest(vector<double> slopes, double& percentError)
{
	const double default_error = 1; // 100% error

	bool set = false;
	double min_index = 0;
	percentError = default_error;

	vector<double> closest(2);
	sort(slopes.begin(), slopes.end());

	// Interate through saving minimum error
	for (unsigned int i = 1; i < slopes.size(); ++i)
	{
		double a = slopes[i-1];
		double b = slopes[i];

		double total = a+b;
		double error;
		
		// They cancel, so they are negatives of each other
		if (total == 0)
			error = default_error;
		else
			error = abs(a-b)/total;

		if (error < percentError)
		{
			set = true;
			min_index = i;
			percentError = error;
		}
	}

	// Return it
	if (set)
	{
		closest[0] = slopes[min_index-1];
		closest[1] = slopes[min_index];
	}
	// Return first 2 elements by default
	else
	{
		if (slopes.size() >= 2)
		{
			closest[0] = slopes[0];
			closest[1] = slopes[1];
		}
	}

	return closest;
}*/

// Find top left and bottom right box. Then, determine slope of these two
// and return the amount to rotate.
double findRotation(Pixels& img, unsigned int& ret_x, unsigned int& ret_y,
	const unsigned int& max_x, const unsigned int& max_y, Image& image)
{
	Coordinate top;
	Coordinate bottom;

	// Set to max large values
	double min_top_dist    = max_y;
	double min_bottom_dist = max_y;

	//
	// The top-left box
	//

	// Goto statements are evil
	bool found = false;

	// Search from top left up a y = x line going down the image
	// Max y+x for also scanning the bottom of the image if shifted to the right
	for (unsigned int z = 0; z < max_y + max_x && !found; ++z)
	{
		for (unsigned int x = 0, y = z; x <= z && x < max_x && !found; ++x, --y)
		{
			// This is an imaginary point (skip till we get to points on the
			// bottom of the image)
			if (y > max_y - 1)
				continue;

			// See if it might be a box
			if (isBlack(img, x, y))
			{
				Coordinate left       = leftmost(img,  x, y, max_x, max_y);
				Coordinate right      = rightmost(img, x, y, max_x, max_y);
				Coordinate midpoint   = midPoint(left, right);
				double apparent_width = distance(left, right);

				// See if the diagonal is about the right length and if a circle in the center
				// of the possible box is almost entirely black
				if (apparent_width <= DIAGONAL+MAX_ERROR && apparent_width >= DIAGONAL-MAX_ERROR &&
					averageColor(img, midpoint.x(), midpoint.y(), BOX_HEIGHT/2, max_x, max_y) > MIN_BLACK)
				{
					double current_top_dist = distance(x, y, 0, 0);

					// See if this box is closer to the top left than the previous one
					if (current_top_dist < min_top_dist)
					{
						min_top_dist = current_top_dist;
						top = Coordinate(x, y);
					}
					// We are getting farther away, so we already found the closest box
					else if (current_top_dist - min_top_dist > MIN_JUMP)
					{
						found = true;
					}
				}
				
				// We only care about the left-most black blob, skip if this is a decent-sized blob
				if (apparent_width > DECENT_SIZE)
					break;
			}
		}
	}

	// 
	// The bottom-left box
	//
	found = false;

	// Start searching at the bottom
	// Stop searching once reaching the y value of the top-left box
	for (unsigned int z = max_y + max_x; z > top.y() && !found; --z)
	{
		for (unsigned int x = 0, y = z; x <= z && x < max_x && !found; ++x, --y)
		{
			// This is an imaginary point (below the bottom)
			if (y > max_y - 1)
				continue;

			// It's black
			if (isBlack(img, x, y))
			{
				Coordinate left       = leftmost(img,  x, y, max_x, max_y);
				Coordinate right      = rightmost(img, x, y, max_x, max_y);
				Coordinate midpoint   = midPoint(left, right);
				double apparent_width = distance(left, right);

				if (right.x()+2 < max_x && right.y()+2 < max_y)
				{
					//image.fillColor("red");
					//image.draw(DrawableRectangle(x, y, x+2, y+2));
					image.fillColor("green");
					image.draw(DrawableRectangle(left.x(), left.y(), left.x()+2, left.y()+2));
					image.fillColor("orange");
					image.draw(DrawableRectangle(right.x(), right.y(), right.x()+2, right.y()+2));
					image.fillColor("red");
					image.draw(DrawableRectangle(midpoint.x(), midpoint.y(), midpoint.x()+2, midpoint.y()+2));
				}

				// It's a box
				if (//apparent_width <= DIAGONAL+MAX_ERROR && apparent_width >= DIAGONAL-MAX_ERROR &&
					averageColor(img, midpoint.x(), midpoint.y(), BOX_HEIGHT/2, max_x, max_y) > MIN_BLACK)
				{

					double current_bottom_dist = distance(left.x(), left.y(), 0, max_y - 1);

					// It's closer than the previous box
					if (current_bottom_dist < min_bottom_dist)
					{
						min_bottom_dist = current_bottom_dist;
						bottom = Coordinate(left.x(), left.y());
					}
					// We're starting to get farther away, so we probably found the closest point
					else if (current_bottom_dist - min_bottom_dist > MIN_JUMP)
					{
						found = true;
					}
				}
				
				// We only care about the left-most black blob, skip if this is a decent-sized blob
				if (apparent_width > DECENT_SIZE)
					break;
			}
			
			/*if (x+1 < max_x && y+1 < max_y)
			{
				PixelPacket *pixel = img.get(x, y, 1, 1);
				*pixel = Color("black");
			}*/
		}
	}

	img.sync();

	// Determine angle from slope of line going through those two boxes
	unsigned int x1 = top.x();
	unsigned int y1 = top.y();
	unsigned int x2 = bottom.x();
	unsigned int y2 = bottom.y();

	ret_x = top.x();
	ret_y = top.y();

	double angle;
	
	// If denominator is zero, don't rotate
	if (y1 != y2)
		angle = atan((1.0*x2 - x1)/(1.0*y2 - y1));
	else
		angle = 0;

	image.fillColor("pink");
	image.draw(DrawableRectangle(x1-10, y1-10, x1-2, y1-2));
	image.draw(DrawableRectangle(x2-10, y2-10, x2-2, y2-2));

	return angle;
}

// Find boxes after rotating, verify they are similar distance from edge
vector< vector<unsigned int> > findRealBoxes(Pixels& img,
	const unsigned int& max_x, const unsigned int& max_y,
	unsigned int& box_width, Image& image)
{
	vector< vector<unsigned int> > boxes = findBoxes(img, max_x, max_y, box_width, image);
	vector< vector<unsigned int> > realBoxes;

	unsigned int firstDist = 0;
	
	if (boxes.size() > 0)
		firstDist = boxes[0][0];

	for (unsigned int i = 0; i < boxes.size(); ++i)
	{
		// Distance from left edge
		unsigned int dist = boxes[i][0];

		unsigned int x1 = boxes[i][0];
		unsigned int y1 = boxes[i][1];
		unsigned int x2 = boxes[i][2];
		unsigned int y2 = boxes[i][3];

		if (abs(dist-firstDist) < box_width)
		{
			realBoxes.push_back(boxes[i]);

			image.fillColor("green");
			image.draw(DrawableRectangle(x1,y1,x1+5,y1+5));
			image.fillColor("purple");
			image.draw(DrawableRectangle(x2,y2,x2+5,y2+5));
		}
		else
		{
			image.fillColor("red");
			image.draw(DrawableRectangle(x1,y1,x1+5,y1+5));
			image.fillColor("orange");
			image.draw(DrawableRectangle(x2,y2,x2+5,y2+5));
		}

	}

	return realBoxes;
}
