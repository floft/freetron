/*
 * Options for the program
 */

#ifndef H_OPTIONS
#define H_OPTIONS

const double min_black = 0.7;		// Minimum value considered black (0 is white, 1 is black)
const double answer_black = 0.2;	// Minimum value considered black for pencil markings
const unsigned int max_missed = 5;	// Stop searching for corner of box after # of white pixels
const unsigned int slope_count = 7;	// Compute slope for # of boxes
const double max_error = 0.3;		// Max error in distance from top left to top right
//const double max_deviation = 0.05;	// Max deviation between slope_count slopes

// Run relative_r(x,y) for r and relative_box(x,y) for box_size
const unsigned int rel_r = 5;		// Determine avg color based on circle of radius r
const unsigned int rel_box_size = 15;	// Determine avg color of boxes based on circle of radius r

// Set r and box_size to image size
inline unsigned int relative_r(const unsigned int& max_x, const unsigned int& max_y)
{ return max_x*rel_r/2500; }
inline unsigned int relative_box(const unsigned int& max_x, const unsigned int& max_y)
{ return max_x*rel_box_size/2500; }

#endif
