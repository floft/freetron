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
    
    const Outline shape(img, blobs, point, MAX_ITERATIONS);

    // Not a valid box if it was beyond max length
    if (!shape.good())
        return;

    const std::vector<Coord> outline = shape.points();

    // Find the four corners by finding the four farthest points from each other
    const Coord p1 = farthestFromPoint(point, outline);
    const Coord p2 = farthestFromPoint(p1,    outline);
    const Coord p3 = farthestFromLine(p1, p2, outline);
    const Coord p4 = farthestFromPoint(p3,    outline);

    // We know p1 and p2 are on opposite corners, so we can use this as a diagonal
    // to find the approximate center
    const Coord center = Coord((p1.x + p2.x)/2, (p1.y + p2.y)/2);
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
    w  = distance(topleft, topright);

    // Find the relative rectangle error
    double rect_error = RECT_ERROR*w;

    // Check if all the points are on the lines between TL-TR, TR-BR, etc.
    for (const Coord& point : outline)
    {
        if (distance(topleft,    topright,    point) > rect_error &&
            distance(topright,   bottomright, point) > rect_error &&
            distance(bottomleft, bottomright, point) > rect_error &&
            distance(topleft,    bottomleft,  point) > rect_error)
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

    if (DEBUG)
    {
        img.mark(topleft, 1);
        img.mark(topright, 1);
        img.mark(bottomleft, 1);
        img.mark(bottomright, 1);
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
