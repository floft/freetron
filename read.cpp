#include "read.h"

// Go right from (x,y) till circle of size BOX_HEIGHT
vector<unsigned int> findFilled(Pixels& img,
	const unsigned int& x,      const unsigned int& y,
	const unsigned int& stop_x, const unsigned int& max_y,
	Image& image)
{
	vector<unsigned int> position;

	// Search to right until hitting circle, jump BOX_WIDTH and continue till stop_x
	for (unsigned int search_x = x; search_x < stop_x; search_x+=BUBBLE_JUMP)
	{
		if (averageColor(img, search_x, y, BOX_HEIGHT, stop_x, max_y) > MIN_ANSWER_BLACK)
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
	Image& image)
{
	unsigned int id = 0;
	map<unsigned int, unsigned int> filled;

	// ID is boxes 2 - 11
	for (unsigned int i = 1; i < 11 && i < boxes.size(); ++i)
	{
		// move over to the first bubble
		unsigned int x = boxes[i].x + FIRST_JUMP;
		// start from the middle y value of the box
		unsigned int y = boxes[i].y;

		// first bubble + 10 bubbles
		unsigned int stop_x = x + 10*BUBBLE_JUMP;

		vector<unsigned int> position = findFilled(img, x, y, stop_x, max_y, image);

		for (unsigned int j = 0; j < position.size(); ++j)
		{
			unsigned int rounded = position[j]/BOX_WIDTH;

			// at x = position, the value is box # - 1 (0 = box 2);
			filled.insert(pair<unsigned int, unsigned int>(rounded, i-1));
		}
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
