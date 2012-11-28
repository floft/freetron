#include "read.h"

// See if the boxes are skewed
bool vertical(const vector<Coord>& boxes,
	const unsigned int start_box, const unsigned int end_box)
{
	// (start_box-1)+1 = start_box
	for (unsigned int i = start_box; i < boxes.size() && i < end_box; ++i)
		if (boxes[i].x > boxes[i-1].x+MAX_ERROR || boxes[i].x < boxes[i-1].x-MAX_ERROR)
			return false;
	
	return true;
}

double answerBlack(Pixels& img, const vector<Coord>& boxes,
	const unsigned int start_box, const unsigned int end_box,
	const unsigned int start_x, const unsigned int stop_x,
	const unsigned int max_y,
	const unsigned int box_width, const unsigned int bubble_jump)
{
	vector<double> colors;
	const unsigned int box_height = box_width/ASPECT;

	// Add color of each row for each box
	for (unsigned int i = start_box-1; i < end_box && i < boxes.size(); ++i)
		for (unsigned int search_x = start_x; search_x <= stop_x; search_x+=bubble_jump)
			colors.push_back(averageColor(img, search_x, boxes[i].y, box_height, stop_x, max_y));
	
	const double max = max_value(colors);
	const double avg = average(colors);

	// If nothing is filled in, don't try to find the values
	if (max < GRAY_SHADE)
		return GRAY_SHADE;
	else
		return (max+avg)/2;
}

// Go right from (x,y) looking for circle of color greater than answer_black
vector<unsigned int> findFilled(Pixels& img,
	const unsigned int x, const unsigned int y,
	const unsigned int stop_x, const unsigned int max_y,
	const unsigned int box_width, const unsigned int bubble_jump,
	const double answer_black)
{
	vector<unsigned int> position;
	const unsigned int box_height = box_width/ASPECT;

	for (unsigned int search_x = x; search_x <= stop_x; search_x+=bubble_jump)
	{
		if (averageColor(img, search_x, y, box_height, stop_x, max_y) > answer_black)
		{
			position.push_back(search_x);

			/*if (DEBUG)
			{
				image.fillColor("green");
				image.draw(DrawableRectangle(search_x-5,y-5,search_x+5,y+5));
			}*/
		}
		/*else if (DEBUG)
		{
			image.fillColor("red");
			image.draw(DrawableRectangle(search_x-5,y-5,search_x+5,y+5));
		}*/
	}

	return position;
}

// Find ID number from card
unsigned int findID(Pixels& img, const vector<Coord>& boxes,
	const unsigned int max_x, const unsigned int max_y,
	const unsigned int box_width)
{
	unsigned int id = 0;
	map<unsigned int, unsigned int> filled;
	
	// ID is boxes 2-11 (counting from 1)
	const unsigned int& start_box = ID_START;
	const unsigned int& end_box   = ID_END;

	// If the boxes don't exist, ...
	if (boxes.size() < end_box)
		return 0;

	// Return 0 if the boxes are skewed
	if (!vertical(boxes, start_box, end_box))
		return 0;

	// Calculate relative values, use ID height to be more acurate
	const unsigned int id_height = boxes[end_box-1].y - boxes[start_box-1].y;
	const unsigned int first_jump  = 1.0*FIRST_JUMP/ID_HEIGHT*id_height;
	const unsigned int bubble_jump = 1.0*BUBBLE_JUMP/ID_HEIGHT*id_height;

	// Since it's vertical, just use the first box
	const unsigned int start_x = boxes[start_box-1].x + first_jump;
	const unsigned int stop_x  = start_x + bubble_jump*(end_box-start_box);

	// Need 75% of max black to be an answer
	const double answer_black = answerBlack(img, boxes, start_box, end_box, start_x, stop_x,
		max_y, box_width, bubble_jump);

	// ID is boxes 2 - 11
	for (unsigned int i = start_box-1; i < end_box && i < boxes.size(); ++i)
	{
		vector<unsigned int> position = findFilled(img, start_x, boxes[i].y, stop_x, max_y,
			box_width, bubble_jump, answer_black);

		// at x = position, the value is box # - 1 (0 = box 2);
		for (const unsigned int pos : position)
			filled[pos] = i-1;
	}

	// Get ID number from map
	int i;
	map<unsigned int, unsigned int>::const_iterator iter;

	for (i = filled.size()-1, iter = filled.begin(); iter != filled.end(); ++iter, --i)
	{
		// Add with zeros for place value
		id += (iter->second)*pow(10, i);
	}

	return id;
}
