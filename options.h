/*
 * Options for the program. We don't want any magic numbers.
 */

#ifndef H_OPTIONS
#define H_OPTIONS

#include <vector> // For size_type

// Allow easy debugging. When true we will save debug0.png, debug1.png, etc. for the
// PDF pages processed with marks of size MARK_SIZE and color MARK_COLOR.
static const bool DEBUG = true;
static const int MARK_SIZE = 1;
static const unsigned char MARK_COLOR = 127;

// The aspect ratio of the black boxes calculated from 49/18, width/height.
// This is used to verify that we have a valid box.
static const double ASPECT = 2.722;

// Minimum percent of pixels in a supposed box needing to be black to be
// considered a box. This is between 0 and 1.
static const double MIN_BLACK = 0.9;

// Average RGB considered black. Currently the colors are stored in a vector
// of chars, so 128 is 50% of 256. (Then -1 since it's zero-based.)
static const int GRAY_SHADE = 127;

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
static const int DIAG_ERROR = 15;

// These values are used to make sure we'll get decent sized boxes. Since we're using the aspect
// ratio and rounding, if we're dealing with a 1px wide box, the height will be rounded to 1px
// most likely, which is within error margins. We want to throw out obviously too large and too
// small objects.
static const int MIN_DIAG = 40;
static const int MAX_DIAG = 150;

// This is a fail-safe for walking the edge of a box. There are occasions when we'll get into
// an infinite loop, so give up after a certain number of turns beyond what a normal box would
// have.
static const int MAX_TURNS = 15;

// To speed up calculations, we save the approximate diagonal of valid boxes so we known when we've
// gone way farther than a real box and give up on large objects. But, we need to verify we have
// good boxes to find the diagonal using. We must have this many close box diagonals to set the
// approximate diagonal.
static const std::vector<int>::size_type DIAG_COUNT = 5;

// The number of previous pixel movements to keep track of when walking around the
// edge of a box. These previous movements are used to see what general direction
// we are walking and thus when we turn a corner. We'll use this to verify the object
// has 4 corners.
static const int PIXEL_RECALL = 5;

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
