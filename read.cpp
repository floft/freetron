#include "read.h"

// Go right from (x,y) till circle of size box_size
vector<unsigned int> findFilled(Pixels& img,
	const unsigned int& x,      const unsigned int& y,
	const unsigned int& stop_x, const unsigned int& max_y,
	const unsigned int& box_width, Image& image)
{
	vector<unsigned int> position;
	vector<unsigned int> regions;


	// Search to right until hitting circle, jump box_size and continue till max_x
	for (unsigned int search_x = x; search_x < stop_x; ++search_x)
	{
		if (averageColor(img, search_x, y, box_width, stop_x, max_y) > MIN_BLACK)
		{
			unsigned int rounded = x + round(search_x-x, box_width);
			position.push_back(rounded);

			image.fillColor("green");
			image.draw(DrawableRectangle(search_x,y,search_x+15,y+15));
			image.fillColor("red");
			image.draw(DrawableRectangle(rounded,y,rounded+15,y+15));
		}
	}

	position.erase(unique(position.begin(), position.end()), position.end());

	return position;
}

// Find ID number from card
unsigned int findID(Pixels& img, const vector< vector<unsigned int> >& boxes,
	const unsigned int& max_x, const unsigned int& max_y,
	const unsigned int& box_width, Image& image)
{
	unsigned int id = 0;
	map<unsigned int, unsigned int> filled;

	// ID is boxes 2 - 11
	for (unsigned int i = 1; i < 11 && i < boxes.size(); ++i)
	{
		// 1 pixel to the right of top right coordinate
		unsigned int x = boxes[i][2]+box_width;
		unsigned int y = boxes[i][3];

		unsigned int stop_x = x + 15*box_width;

		vector<unsigned int> position = findFilled(img, x, y, stop_x, max_y, box_width, image);

		for (unsigned int j = 0; j < position.size(); ++j)
		{
			unsigned int rounded = position[j]/box_width;

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
