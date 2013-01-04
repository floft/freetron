/*
 * Code to determine the rotation of the image
 */

#ifndef H_ROTATE
#define H_ROTATE

#include <cmath>
#include <vector>
#include <algorithm>

#include "options.h"
#include "pixels.h"
#include "math.h"

// Find top-left and bottom-left boxes and give rotation to make them vertical
// TODO: make this const
//double findRotation(Pixels& img, Coord& ret_coord, BoxData* data);

// Rotate the image by finding a line at least ROTATE_LEN pixels long
double findRotation(Pixels& img, Coord& ret_coord);

int goUp(const Pixels& img, const Coord& p, const Coord& orig);
int goLeft(const Pixels& img, const Coord& p, const Coord& orig);
int goDown(const Pixels& img, const Coord& p, const Coord& orig);
int goRight(const Pixels& img, const Coord& p, const Coord& orig);

Coord findBottom(const Pixels& img, const Coord& point);

#endif
