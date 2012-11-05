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

// Determine average slope of 2 boxes if they are close enough, otherwise
// continue till finding closer boxes
double findRotation(Pixels& img, const vector< vector<unsigned int> >& boxes,
	unsigned int& ret_x, unsigned int& ret_y,
	const unsigned int& max_x, const unsigned int& max_y, Image& image)
{
	//double slope = 0;
	//unsigned int offset = 0; // Start with the first box

	/*while (!set && offset < boxes.size())
	{
		vector<double> slopes;

		for (unsigned int i = boxes.size()-offset-1; i > boxes.size()-offset-slope_count-1; --i)
		{
			unsigned int x1 = boxes[i][0];
			unsigned int y1 = boxes[i][1];
			unsigned int x2 = boxes[i][2];
			unsigned int y2 = boxes[i][3];
			
			// Rotate around this point
			ret_x = x1;
			ret_y = y1;

			slopes.push_back(1.0*(y2 - y1)/(x2 - x1));
		}

		double average;

		// If at least 2 are very close, set slope to average of the closest
		if (stdDev(slopes, average) < max_deviation)
		{
			set = true;
			slope = average;
		}
		else
		{
			++offset;
		}
	}*/

	// Die quickly
	if (boxes.size() == 0)
	{
		ret_x = 0;
		ret_y = 0;
		return 0;
	}
	
	unsigned int top = 0;
	unsigned int bottom = 0;
	// Set to max large values
	double top_dist = max_y;
	double bottom_dist = max_y;

	// Find top and bottom boxes
	for (unsigned int i = 0; i < boxes.size(); ++i)
	{
		double top_left    = distance(boxes[i][0], boxes[i][1], 0, 0);
		double bottom_left = distance(boxes[i][0], boxes[i][1], 0, max_y - 1);

		// Top left
		if (top_left < top_dist)
		{
			top = i;
			top_dist = top_left;
		}
		
		// Bottom right
		if (bottom_left < bottom_dist)
		{
			bottom = i;
			bottom_dist = bottom_left;
		}
	}
	
	// Determine angle from slope of line going through those two boxes
	unsigned int x1 = boxes[top][0];
	unsigned int y1 = boxes[top][1];
	unsigned int x2 = boxes[bottom][0];
	unsigned int y2 = boxes[bottom][1];

	//cout << "(" << x1 << "," << y1 << ")" << endl;
	//cout << "(" << x2 << "," << y2 << ")" << endl;

	ret_x = boxes[top][0];
	ret_y = boxes[top][1];
	double angle = atan((1.0*x2 - x1)/(1.0*y2 - y1));

	//image.fillColor("pink");
	//image.draw(DrawableRectangle(x1-10, y1-10, x1-2, y1-2));
	//image.draw(DrawableRectangle(x2-10, y2-10, x2-2, y2-2));

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
