/*
 * Options for the program
 */

#ifndef H_OPTIONS
#define H_OPTIONS

#include <vector> // For size_type

// Let's avoid comparing signed and unsigned integers
typedef std::vector<int>::size_type vsize;

const bool DEBUG = true;		// Output test image
const int  MARK_SIZE = 10;		// Size of lines output in debug image
const unsigned char MARK_COLOR = 127;	// Grey color

const double ASPECT      = 2.722;	// Aspect ratio of black boxes Width/Height (calculated with 49/18)
const double MIN_BLACK   = 0.75;	// Minimum percent (0-1) of pixels black in circle to be considered a box
const int    GRAY_SHADE  = 127;		// Average RGB considered black
const int    MAX_ERROR   = 5;		// Max error in pixels
const int    MIN_JUMP    = 500;		// Increase in distance to top or bottom left needed to give up searching for closer box (better too high than too low)
const int    EDGE_JUMP   = 15;		// If we've gone beyond this many pixels from being closer to Top left for instance, we've found the corner
const int    DECENT_SIZE = 15;		// Continue searching for a box if only encountering black specs less than 5 pixels wide
const int    MAX_DIAG    = 150;		// Used as a check to verify we have decent values, if failed and correct, it'll just take a bit longer
const int    MIN_DIAG    = 40;		// It will probably never be less than this, really just needs to be above 10 or so to get rid of rounding problems
const int    DARK_RAD    = 5;		// "Radius" of square to check around a point to find a region that is more black
const vsize  DIAG_COUNT  = 5;		// Calculate diagonal if at least 5 are within error margins (vsize since always used as: vector.size() > DIAG_COUNT)


// Relative values
const int ID_HEIGHT   = 452;		// Height from box 2 to box 11, used for *_JUMP
const int FIRST_JUMP  = 115;		// Number of pixels right of box is the first bubble, relative to height
const int BUBBLE_JUMP = 75;		// Number of pixels to jump from bubble to bubble, relative to height

// Box numbers for ID section
const int ID_START = 2;
const int ID_END   = 11;

// Threading
const int THREAD_WAIT = 10;		// Check if we can create a new thread every few milliseconds

#endif
