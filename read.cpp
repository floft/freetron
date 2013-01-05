#include "read.h"

// Generate bubble from width, BUBBLE_CURVE, BUBBLE_ASPECT, and WHITE_SEARCH
std::vector<std::vector<bool>> genBubble(int box_width, int box_height)
{
	int width  = box_width;
	int height = smartCeil(box_width/BUBBLE_ASPECT);

	int real_width  = width  + WHITE_SEARCH*2;
	int real_height = height + WHITE_SEARCH*2;

	std::vector<std::vector<bool>> bubble(real_height, std::vector<bool>(real_width, false));

	// Make box in middle of bubble black
	const int curve_height = (height - box_height)/2;
	const int box_start_y = WHITE_SEARCH + curve_height;
	const int box_end_y   = real_height - box_start_y;
	const int box_start_x = WHITE_SEARCH;
	const int box_end_x   = real_width - WHITE_SEARCH;

	for (int y = box_start_y; y < box_end_y; ++y)
		for (int x = box_start_x; x < box_end_x; ++x)
			bubble[y][x] = true;

	// Add box sections in between two curved corners
	//const int curve_width =

	// TODO: finish this

	return bubble;
}

// See if the boxes are skewed
bool vertical(const std::vector<Coord>& boxes,
	const int start_box, const std::vector<Coord>::size_type end_box)
{
	typedef std::vector<Coord>::size_type size_type;

	// (start_box-1)+1 = start_box
	for (size_type i = start_box; i < boxes.size() && i < end_box; ++i)
		if (boxes[i].x > boxes[i-1].x+MAX_ERROR || boxes[i].x < boxes[i-1].x-MAX_ERROR)
			return false;
	
	return true;
}

double answerBlack(const Pixels& img, const std::vector<Coord>& boxes,
	const int start_box, const std::vector<Coord>::size_type end_box,
	const int start_x, const int stop_x,
	const int box_width, const int bubble_jump)
{
	typedef std::vector<Coord>::size_type size_type;

	std::vector<double> colors;
	const int box_height = box_width/ASPECT;

	// For each row, find max value
	for (size_type i = start_box-1; i < end_box && i < boxes.size(); ++i)
	{
		std::vector<double> row;

		for (int search_x = start_x; search_x <= stop_x; search_x+=bubble_jump)
			// box_height is approximately the radius of the bubble if it was a circle
			row.push_back(averageColor(img, search_x, boxes[i].y, box_height));

		const double row_max = max_value(row);
		
		colors.push_back(row_max);
	}

	const double max = max_value(colors);
	const double min = min_value(colors);
	const double avg = average(colors);
	
	// TODO: this works, but is completely luck. It works because there are blank
	// rows. If somebody has the ID 0123456789, then this will error.
	
	// See if this is at least a certain multiple of the whitest circle
	if (max/min < 1.5)
		return GRAY_SHADE;
	else
		return avg;
}

// Go right from (x,y) looking for circle of color greater than answer_black
std::vector<int> findFilled(Pixels& img,
	const int x, const int y,
	const int stop_x,
	const int box_width, const int bubble_jump,
	const double answer_black)
{
	std::vector<int> position;
	const int box_height = box_width/ASPECT;

	for (int search_x = x; search_x <= stop_x; search_x+=bubble_jump)
	{
		if (averageColor(img, search_x, y, box_height) > answer_black)
		{
			position.push_back(search_x);

			//std::cout << averageColor(img, search_x, y, box_height, stop_x, max_y) << " > " << answer_black << std::endl;

			if (DEBUG)
				img.mark(Coord(search_x, y));
		}
	}

	return position;
}

// Find ID number from card
int findID(Pixels& img, const std::vector<Coord>& boxes, BoxData& data)
{
	typedef std::vector<Coord>::size_type size_type;

	int id = 0;
	std::map<int, int> filled;
	
	// ID is boxes 2-11 (counting from 1)
	const size_type& start_box = ID_START;
	const size_type& end_box   = ID_END;

	// If the boxes don't exist, ...
	if (boxes.size() < end_box)
		return 0;

	// Return 0 if the boxes are skewed
	if (!vertical(boxes, start_box, end_box))
		return 0;

	// Calculate relative values, use ID height to be more acurate
	const int id_height   = boxes[end_box-1].y - boxes[start_box-1].y;
	const int first_jump  = 1.0*FIRST_JUMP/ID_HEIGHT*id_height;
	const int bubble_jump = 1.0*BUBBLE_JUMP/ID_HEIGHT*id_height;

	// Since it's vertical, just use the first box
	const int start_x = boxes[start_box-1].x + first_jump;
	const int stop_x  = start_x + bubble_jump*(end_box-start_box);

	// Calculate value needed to be considered black from values
	const double answer_black = answerBlack(img, boxes, start_box, end_box, start_x, stop_x,
		data.width, bubble_jump);

	// ID is boxes 2 - 11
	for (size_type i = start_box-1; i < end_box && i < boxes.size(); ++i)
	{
		std::vector<int> position = findFilled(img, start_x, boxes[i].y, stop_x, 
			data.width, bubble_jump, answer_black);

		// at x = position, the value is box # - 1 (0 = box 2);
		for (const int pos : position)
			filled[pos] = i-1;
	}

	// Get ID number from map
	int i;
	std::map<int, int>::const_iterator iter;

	for (i = filled.size()-1, iter = filled.begin(); iter != filled.end(); ++iter, --i)
	{
		// Add with zeros for place value
		id += (iter->second)*pow(10, i);
	}

	return id;
}
