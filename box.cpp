#include "box.h"

// Find square around point
Square::Square(const Pixels& img, const int x, const int y, const int r)
{
    const int x1 = std::min((x<=r)?0:x-r, img.width()-1);
    const int y1 = std::min((y<=r)?0:y-r, img.height()-1);
    topleft      = Coord(x1, y1);

    const int x2 = std::min(x+r, img.width()-1);
    const int y2 = std::min(y+r, img.height()-1);
    bottomright  = Coord(x2, y2);

    const int mid_x = (x1+x2)/2;
    const int mid_y = (y1+y2)/2;
    midpoint        = Coord(mid_x, mid_y);
}

// Useful for debugging
bool Square::in(const Coord& c) const
{
    return (c.x >= topleft.x && c.x <= bottomright.x &&
            c.y >= topleft.y && c.y <= bottomright.y);
}

// Find box properties (corners, width, height, aspect ratio, mid point, etc.)
Box::Box(Pixels& img, const Blobs& blobs, const Coord& point)
    :img(img), blobs(blobs)
{
    label = blobs.label(point);

    if (label == Blobs::default_label)
    {
        log("box can't be created from default label");
        return;
    }
    
    const Outline shape(blobs, point, MAX_ITERATIONS);

    // Not a valid box if it was beyond max length
    if (!shape.good())
        return;

    const std::vector<Coord> outline = shape.points();

    // Find the four corners by finding the four farthest points from each other.
    // Note that we'll use the square versions of these, since this is a box
    // instead of a elipse; otherwise, slight extrusions on the side of a box
    // might throw the corners off.
    const Coord p1 = farthestFromPointSquare(point, outline);
    const Coord p2 = farthestFromPointSquare(p1, outline);

    // We know p1 and p2 are on opposite corners, so we can use this as a diagonal
    // to find the approximate center
    const Coord center = findMidpoint(p1, p2);

    // Calculate other two points. We can't use the same algorithm above. We'd
    // have to calculate the farthest points perpendicularly from the line from
    // p1 to p2, but if it is a box, we could just rotate these two points by the
    // known box aspect ratio. In tests this works out in just a few more cases.
    double rotation = 2*std::atan(1/ASPECT);

    // For determing which way to rotate, we'll look at the sign of an angle.
    // This requires that we have a consistent order of points.
    Coord bottom = p2;
    Coord top = p1;

    if (p1.y > p2.y)
        std::swap(bottom, top);

    // Rotate clockwise if current diagonal BL/TR, counterclockwise if BR/TL
    if (std::atan(1.0*(bottom.y-top.y)/(bottom.x-top.x)) < 0)
        rotation *= -1.0;

    const double sin_rad = std::sin(rotation);
    const double cos_rad = std::cos(rotation);
    Coord p3 = img.rotatePoint(center, p1, sin_rad, cos_rad);
    Coord p4 = img.rotatePoint(center, p2, sin_rad, cos_rad);

    // If we have invalid points, try this other method. It'll work in most cases.
    // The only time it shouldn't work is if it's rotated more than say 25 degrees.
    // An alternative to this would be farthestFromLine operating on the half of
    // outline between p1 and p2 on one side of the box and then on the other side
    // (based on the index with some sort of wrap-around).
    if (p3 == default_coord || p4 == default_coord)
    {
        // farthestFromLine doesn't have the x+y style algorithm tending towards
        // boxes, so use it after the perpendicular from line algorithm to get a
        // more likely corner. Otherwise this third point is not as good of a corner
        // as the other three, which occasionally will throw out the box.
        p3 = farthestFromPointSquare(farthestFromLine(p1, p2, outline), outline);
        p4 = farthestFromPointSquare(p3, outline);
    }

    const Coord* const points[] = { &p1, &p2, &p3, &p4 };

    // Top left is up and left, ...
    for (const Coord* const point : points)
    {
        // Bet you never saw this style of formatting before =)
             if (point->x < center.x && point->y < center.y)
            topleft     = *point;
        else if (point->x > center.x && point->y < center.y)
            topright    = *point;
        else if (point->x < center.x && point->y > center.y)
            bottomleft  = *point;
        else
            bottomright = *point;
    }

    // We need the width to check for out-of-line points
    w = distance(topleft, topright);

    // Find the relative rectangle error
    double rect_error = RECT_ERROR*w;

    // Check if all the points are on the lines between TL-TR, TR-BR, etc.
    // If a point is farther from all of these lines than a relative error,
    // it's not a box.
    for (const Coord& testPoint : outline)
    {
        if (distance(topleft,    topright,    testPoint) > rect_error &&
            distance(topright,   bottomright, testPoint) > rect_error &&
            distance(bottomleft, bottomright, testPoint) > rect_error &&
            distance(topleft,    bottomleft,  testPoint) > rect_error)
            return;
    }

    // We've traced some shape now, so using the four supposed "corners" calculate the
    // box's properties
    h  = distance(topleft, bottomleft);
    mp = findMidpoint(topleft, bottomright);
    ar = (h>0)?1.0*w/h:0;
    diag = std::ceil(std::sqrt(w*w+h*h));

    // What should the width be approximately given the aspect ratio (width/height)
    const double approx_height = w/ASPECT;

    // Note that when using the rotation method for calculating p3 and p4 above,
    // the first "Correct height" check is more or less pointless since the aspect
    // ratio will be exactly correct. In this case we'll rely more on other checks
    // like validBoxColor().
    if (h >= approx_height-HEIGHT_ERROR && h <= approx_height+HEIGHT_ERROR && // Correct height
        std::abs(distance(topleft, bottomright) - distance(topright, bottomleft)) < DIAG_ERROR && // Diagonals same
        std::abs(distance(topleft, bottomleft) - distance(topright, bottomright)) < DIAG_ERROR && // Height same
        std::abs(distance(topleft, topright) - distance(bottomleft, bottomright)) < DIAG_ERROR && // Width same
        std::abs(slopeXY(topleft, bottomleft) - slopeXY(topright, bottomright)) < SLOPE_ERROR_HEIGHT && // Height slope
        std::abs(slopeYX(topleft, topright) - slopeYX(bottomleft, bottomright)) < SLOPE_ERROR_WIDTH  && // Width slope
        diag >= MIN_DIAG && diag <= MAX_DIAG && // Get rid of 1-5px "boxes"
        validBoxColor()) // Black enough in box and white enough around box
    {
        if (DEBUG)
        {
            img.mark(topleft);
            img.mark(topright);
            img.mark(bottomleft);
            img.mark(bottomright);

            for (const Coord& c : outline)
                img.mark(c, 1);
        }

        valid_box = true;
    }
}

// Get average color of pixels within the corners of the box and also the average
// color of the pixels WHITE_SEARCH outside of the box.
bool Box::validBoxColor() const
{
    int inside_black  = 0;
    int inside_total  = 0;
    int around_black = 0;
    int around_total = 0;

    // Points of box WHITE_SEARCH larger this box
    const Coord tl = Coord(topleft.x-WHITE_SEARCH,
        lineFunctionY(topleft, mp, topleft.x-WHITE_SEARCH));
    const Coord bl = Coord(bottomleft.x-WHITE_SEARCH,
        lineFunctionY(bottomleft, mp, bottomleft.x-WHITE_SEARCH));
    const Coord tr = Coord(topright.x+WHITE_SEARCH,
        lineFunctionY(topright, mp, topright.x+WHITE_SEARCH));
    const Coord br = Coord(bottomright.x+WHITE_SEARCH,
        lineFunctionY(bottomright, mp, bottomright.x+WHITE_SEARCH));

    Square bounds(img, mp.x, mp.y, (br.x - tl.x)/2);

    for (int y = bounds.topLeft().y; y <= bounds.bottomRight().y; ++y)
    {
        for (int x = bounds.topLeft().x; x <= bounds.bottomRight().x; ++x)
        {
            // Inside box
            if (y > lineFunctionY(topleft,    topright,    x) &&
                y < lineFunctionY(bottomleft, bottomright, x) &&
                x > lineFunctionX(topleft,    bottomleft,  y) &&
                x < lineFunctionX(topright,   bottomright, y))
            {
                if (blobs.label(Coord(x,y)) == label)
                    ++inside_black;

                ++inside_total;
            }
            // In area around box
            else if (y >= lineFunctionY(tl, tr, x) &&
                 y <= lineFunctionY(bl, br, x) &&
                 x >= lineFunctionX(tl, bl, y) &&
                 x <= lineFunctionX(tr, br, y))
            {
                if (blobs.label(Coord(x,y)) == label)
                    ++around_black;

                ++around_total;
            }
        }
    }

    // Percentage black inside box and around box
    const double inside = (inside_total>0)?(1.0*inside_black/inside_total):0;
    const double around = (around_total>0)?(1.0*around_black/around_total):0;

    return (inside > MIN_BLACK && around < MAX_BLACK);
}
