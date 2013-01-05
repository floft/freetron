/*
 * Options for the program. We don't want any magic numbers.
 */

#ifndef H_OPTIONS
#define H_OPTIONS

#include <string>
#include <vector>

#include "data.h"

typedef std::vector<int>::size_type   vsize_int;
typedef std::vector<Coord>::size_type vsize_coord;

// Allow easy debugging. When true we will save debug0.png, debug1.png, etc. for the
// PDF pages processed with marks of size MARK_SIZE and color MARK_COLOR.
static const bool DEBUG = true;
static const int MARK_SIZE = 1;
static const unsigned char MARK_COLOR = 127;

// Whether to write to a log file, and the file we will write to if true. If enabled,
// the log messages written to the screen in debug mode will be written to this file.
static const bool LOGGING = true;
static std::string LOG_FILE = "freetron.log";

// The aspect ratio of the black boxes calculated from 49/18, width/height.
// This is used to verify that we have a valid box.
static const double ASPECT = 2.722;

// Minimum percent of pixels in a supposed box needing to be black to be
// considered a box. This is between 0 and 1.
static const double MIN_BLACK = 0.75;

// Maximum percent of pixels that can be black in the region around a box, and
// what sized region around box to check in pixels.
static const double MAX_BLACK = 1-MIN_BLACK;
static const int WHITE_SEARCH = 5;

// Average RGB considered black. Currently the colors are stored in a vector
// of chars, so 128 is 50% of 256. (Then -1 since it's zero-based.)
static const int GRAY_SHADE = 127;

// When finding the filled in bubbles, look for something as wide as a box with a height
// calculated from the below aspect ratio. The curve of the bubble is generated from a
// quarter-circle with it's center BUBBLE_CURVE*width pixels in on each side. Note that
// searching for the bubbles also uses WHITE_SEARCH to look at the surrounding region.
static const double BUBBLE_ASPECT = 1.566; // From 47/30

// The error margin in pixels for still considering this form as potentially valid.
// After we find all boxes, we'll look for the filled in bubbles. If the boxes are
// beyond this far from vertical, we'll give up and say an error occured while processing
// this form.
static const int MAX_ERROR = 5;

// The error margin in pixels for difference in height from the estimated height from
// the aspect ratio and width. If it's beyond this it won't be considered a box.
static const int HEIGHT_ERROR = 5;

// The error margin in pixels for the difference in diagonal from the diagonals of other
// valid boxes. Also used for error beyond diagonal to see if we've definitely gone farther
// than we could go in a box (if displacement from original point is greater than this, it
// won't be a box, so save time and give up).
static const int DIAG_ERROR = 10;

// The error margin for the diffence in slope for the width and height. This is to make sure
// that it is more of a parallelogram instead of just a quadrilateral.
static const double SLOPE_ERROR_WIDTH  = 0.1;	// 5/50 is 0.1
static const double SLOPE_ERROR_HEIGHT = 0.25;	// 5/20 is 0.25

// These values are used to make sure we'll get decent sized boxes. Since we're using the aspect
// ratio and rounding, if we're dealing with a 1px wide box, the height will be rounded to 1px
// most likely, which is within error margins. We want to throw out obviously too large and too
// small objects.
static const int MIN_DIAG = 30;
static const int MAX_DIAG = 150;

// This is a value used to speed up searching for boxes. If the distance between the first and last
// points of an object are greater than MAX_DIAG or less than MIN_HEIGHT (since they may be the width,
// height, or diagonal), we'll skip it.
static const int MIN_HEIGHT = 5;

// As a fail-safe when walking the edge of the potential boxes, we'll give up after moving this many
// pixels just in case something goes wrong (it's a while true loop). Boxes will never be four times
// the diagonal around, so this should be a safe value.
static const int MAX_ITERATIONS = MAX_DIAG*4;

// To speed up calculations, we save the approximate diagonal of valid boxes so we known when we've
// gone way farther than a real box and give up on large objects. But, we need to verify we have
// good boxes to find the diagonal using. We must have this many close box diagonals to set the
// approximate diagonal.
static const vsize_int DIAG_COUNT = 5;

// Look at this many pixel movements when determining if we've changed direction when walking
// around the edge of an object. We'll keep track of the last floor(PIXEL_RECALL/2) and look ahead
// at the next ceil(PIXEL_RECALL/2) so that we detect the change at the current pixel instead of
// a few early or late. We detect the corners so we can verify this is a quadrilateral.
static const int PIXEL_RECALL = 5;

// Total number of boxes, to verify we found the right ones
static const vsize_coord TOTAL_BOXES = 54;

// When determining the student ID, we need to know which boxes correspond to the ID.
static const int ID_START = 2;
static const int ID_END   = 11;

// In the threading header file, we create a certain number of threads depending on the number
// of CPUs. After we create as many threads as cores, we wait this many milliseconds till we check
// if any of the threads have completed.
static const int THREAD_WAIT = 20;

/*
 * TODO: I'm not entirely sure these will still be used, so I won't comment about them yet.
 */

// Relative values
static const int ID_HEIGHT   = 452; // Height from box 2 to box 11, used for *_JUMP
static const int FIRST_JUMP  = 115; // Number of pixels right of box is the first bubble, relative to height
static const int BUBBLE_JUMP = 75;  // Number of pixels to jump from bubble to bubble, relative to height

#endif
