/*
 * Options for the program
 */

#ifndef H_OPTIONS
#define H_OPTIONS

const bool DEBUG = true;		// Output test image

const unsigned int BOX_WIDTH  = 49;	// At 300dpi the box width in pixels
const unsigned int BOX_HEIGHT = 18;	// At 300dpi the box height in pixels (should be smaller than width)
const unsigned int MAX_ERROR = 5;	// Max error in pixels (e.g. box_width plus or minus error)
const unsigned int DIAGONAL = 53;	// ceil(sqrt(box_width^2+box_height^2)) plus or minus error
const double MIN_BLACK = 0.8;		// Minimum percent (0-1) of pixels black in circle to be considered a box
const double MIN_ANSWER_BLACK = 0.5;	// Minimum percent (0-1) of pixels for marking it as an answer
const unsigned int MIN_JUMP = 100;	// Increase in distance to top or bottom left needed to give up searching for closer box
const unsigned int DECENT_SIZE = 15;	// Continue searching for a box if only encountering black specs less than 5 pixels wide
const double GRAY_SHADE = 0.5;		// Average RGB considered black
const unsigned int FIRST_JUMP = 115;	// Number of pixels right of box is the first bubble
const unsigned int BUBBLE_JUMP = 75;	// Number of pixels to jump from bubble to bubble

#endif
