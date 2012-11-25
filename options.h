/*
 * Options for the program
 */

#ifndef H_OPTIONS
#define H_OPTIONS

const bool DEBUG = true;		// Output test image

const double       ASPECT      = 2.722; // Aspect ratio of black boxes Width/Height (calculated with 49/18)
const double       MIN_BLACK   = 0.9;	// Minimum percent (0-1) of pixels black in circle to be considered a box
const double       GRAY_SHADE  = 0.5;	// Average RGB considered black
const unsigned int MAX_ERROR   = 5;	// Max error in pixels (e.g. box_width plus or minus error)
const unsigned int MIN_JUMP    = 500;	// Increase in distance to top or bottom left needed to give up searching for closer box (better too high than too low)
const unsigned int DECENT_SIZE = 15;	// Continue searching for a box if only encountering black specs less than 5 pixels wide
const unsigned int DIAG_COUNT  = 5;	// Calculate diagonal if at least 5 are within error margins
const unsigned int MAX_DIAG    = 150;	// Used as a check to verify we have decent values, if failed and correct, it'll just take a bit longer
const unsigned int MIN_DIAG    = 40;	// It will probably never be less than this, really just needs to be above 10 or so to get rid of rounding problems

// Relative values
const unsigned int ID_HEIGHT   = 452;	// Height from box 2 to box 11, used for *_JUMP
const unsigned int FIRST_JUMP  = 115;	// Number of pixels right of box is the first bubble, relative to height
const unsigned int BUBBLE_JUMP = 75;	// Number of pixels to jump from bubble to bubble, relative to height

// Box numbers for ID section
const unsigned int ID_START = 2;
const unsigned int ID_END   = 11;

#endif
