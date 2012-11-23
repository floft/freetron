#include "read.h"

// Go right from (x,y) till circle of size BOX_HEIGHT
vector<unsigned int> findFilled(Pixels& img,
	const unsigned int& x, const unsigned int& y,
	const unsigned int& stop_x, const unsigned int& max_y,
	const unsigned int& box_height, const unsigned int& bubble_jump, Image& image)
{
	// Find average answer color
	vector<double> colors;

	for (unsigned int search_x = x; search_x < stop_x; search_x+=bubble_jump)
		colors.push_back(averageColor(img, search_x, y, box_height, stop_x, max_y));
	
	double answer_black = average(colors);

	// Search at each bubble for something greater than average
	vector<unsigned int> position;

	for (unsigned int search_x = x; search_x < stop_x; search_x+=bubble_jump)
	{
		if (averageColor(img, search_x, y, box_height, stop_x, max_y) > answer_black)
		{
			position.push_back(search_x);

			if (DEBUG)
			{
				image.fillColor("green");
				image.draw(DrawableRectangle(search_x-5,y-5,search_x+5,y+5));
			}
		}
		else if (DEBUG)
		{
			image.fillColor("red");
			image.draw(DrawableRectangle(search_x-5,y-5,search_x+5,y+5));
		}
	}

	return position;
}

// Find ID number from card
unsigned int findID(Pixels& img, const vector<Coord>& boxes,
	const unsigned int& max_x, const unsigned int& max_y,
	const unsigned int& box_height, Image& image)
{
	unsigned int id = 0;
	map<unsigned int, unsigned int> filled;

	// Calculate relative values
	unsigned int first_jump  = floor(1.0*FIRST_JUMP/BOX_HEIGHT*box_height);
	unsigned int bubble_jump = floor(1.0*BUBBLE_JUMP/BOX_HEIGHT*box_height);

	// ID is boxes 2 - 11
	for (unsigned int i = 1; i < 11 && i < boxes.size(); ++i)
	{
		// move over to the first bubble
		unsigned int x = boxes[i].x + first_jump;
		// start from the middle y value of the box
		unsigned int y = boxes[i].y;

		// first bubble + 10 bubbles
		unsigned int stop_x = x + 10*bubble_jump;

		vector<unsigned int> position = findFilled(img, x, y, stop_x, max_y,
			box_height, bubble_jump, image);

		// at x = position, the value is box # - 1 (0 = box 2);
		for (unsigned int j = 0; j < position.size(); ++j)
			filled.insert(pair<unsigned int, unsigned int>(position[j], i-1));
	}

	// Get ID number from map
	int i;
	map<unsigned int, unsigned int>::iterator iter;

	for (i = filled.size()-1, iter = filled.begin(); iter != filled.end(); ++iter, --i)
	{
		// Add with zeros for place value
		id += (iter->second)*pow(10, i);
	}

	return id;
}
