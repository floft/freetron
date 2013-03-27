#include "box.h"

// The priority for which pixels to go to, search forward if no black yet
// or backward for white if first is black (see Box::findEdge)
//  0 1 2
//  7   3
//  6 5 4
const std::array<Coord, 8> Box::matrix = {{
    Coord(-1, -1),
    Coord( 0, -1),
    Coord( 1, -1),
    Coord( 1,  0),
    Coord( 1,  1),
    Coord( 0,  1),
    Coord(-1,  1),
    Coord(-1,  0)
}};

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

// Average color of all pixels within radius r of (x,y)
// 0 = complete white, 1 = complete black
//
// TODO: make this use the label?
double averageColor(const Pixels& img,
    const int x, const int y,
    const int r)
{
    // Find square around circle of radius r centered at (x,y)
    Square s(img, x, y, r);
    const int x1    = s.topLeft().x;
    const int y1    = s.topLeft().y;
    const int x2    = s.bottomRight().x;
    const int y2    = s.bottomRight().y;
    const int mid_x = s.midPoint().x;
    const int mid_y = s.midPoint().y;

    // Maybe this makes it a bit faster
    const int r2 = r*r;

    int black  = 0;
    int total  = 0;

    for (int search_y = y1; search_y < y2; ++search_y)
    {
        for (int search_x = x1; search_x < x2; ++search_x)
        {
            if (std::pow(std::abs(search_x-mid_x),2) + std::pow(std::abs(search_y-mid_y),2) <= r2)
            {
                if (img.black(Coord(search_x, search_y)))
                    ++black;

                ++total;
            }
        }
    }
    
    if (total > 0)
        return 1.0*black/total;
    else
        return 0;
}

// Find box properties (corners, width, height, aspect ratio, mid point, etc.)
Box::Box(Pixels& img, const Blobs& blobs, const Coord& point, BoxData& data)
    :img(img), blobs(blobs), data(data)
{
    // Current label so we only walk around this object
    label = blobs.label(point);

    if (label == Blobs::default_label)
    {
        log("box can't be created from default label");
        return;
    }
    
    // Keep track of previous points. If we go to one again, give up. Otherwise
    // we'll get into an infinite loop.
    std::vector<Coord> path;

    // Start one pixel above this first point (we wouldn't have been given this
    // point if the pixel above it was black)
    Coord position(point.x, point.y-1);
    
    // End where we started
    Coord end = position;
    
    // Top left and top right coordinates used to find corners
    const Coord origin(0, 0);
    const Coord right(img.width() - 1, 0);

    // The distances from (0,0) and (width-1, 0) used to find corners
    double TL_dist = img.height();
    double TR_dist = img.height();
    double BR_dist = 0;
    double BL_dist = 0;

    // A fail-safe
    int iterations = 0;

    bool test = false;
    if (Square(img, 32, 990, 25).in(point))
        test = true;
    
    // Walk the edge of the box and find the corners using the distances to image corners
    // Note: "break" when we're done, "return" gives up since this isn't a box
    while (true)
    {
        // From the distance to the top left and right points determine if this
        // is a corner of the quadrilateral
        double left_dist  = distance(position, origin);
        double right_dist = distance(position, right);

        if (left_dist < TL_dist)
        {
            topleft = position;
            TL_dist = left_dist;
        }

        if (left_dist > BR_dist)
        {
            bottomright = position;
            BR_dist = left_dist;
        }

        if (right_dist < TR_dist)
        {
            topright = position;
            TR_dist = right_dist;
        }

        if (right_dist > BL_dist)
        {
            bottomleft = position;
            BL_dist = right_dist;
        }

        // Find next pixel to go to
        EdgePair edge = findEdge(position, path);

        // Ran out of options (shouldn't happen?)
        if (edge.index == -1)
        {
            log("ran out of options on edge");
            return;
        }

        // Go to new position (note that edge.point will be position unless
        // we had to retrace our steps a ways)
        position = edge.point+matrix[edge.index];
        path.push_back(position);

        // We've walked around the entire object now
        if (position == end)
            break;
        
        // We're not in a box if we're beyond this
        ++iterations;

        if (iterations > MAX_ITERATIONS)
            return;
    }

    // We've traced some shape now, so using the four "corners," calculate some
    // values we'll later use to see if this is really a box
    w  = distance(topleft, topright);
    h  = distance(topleft, bottomleft);
    mp = Coord((topleft.x + bottomright.x)/2, (topleft.y + bottomright.y)/2);
    ar = (h>0)?1.0*w/h:0;

    // If we haven't returned already, we might be a box
    possibly_valid = true;

    if (test)
    {
        img.mark(topleft);
        img.mark(topright);
        img.mark(bottomleft);
        img.mark(bottomright);

        std::cout << "MP: " << mp << std::endl;
    }
}

bool Box::valid()
{
    // If something when wrong while finding corners, nothing below applies
    if (!possibly_valid)
        return false;
    
    // What should the width be approximately given the aspect ratio (width/height)
    const double approx_height = w/ASPECT;
    const int real_diag = std::ceil(std::sqrt(w*w+h*h));

    if (Square(img, 32, 990, 25).in(mp))
    {
        std::cout << w << " " << h << " " << approx_height << " " << real_diag << " " << data.diag << std::endl;
        std::cout << (h >= approx_height-HEIGHT_ERROR) << " " << (h <= approx_height+HEIGHT_ERROR) << " "
                  << (std::abs(distance(topleft, bottomright) - distance(topright, bottomleft)) < DIAG_ERROR) << " "
                  << (std::abs(distance(topleft, bottomleft) - distance(topright, bottomright)) < DIAG_ERROR) << " "
                  << (std::abs(distance(topleft, topright) - distance(bottomleft, bottomright)) < DIAG_ERROR) << " "
                  << (std::abs(slopeXY(topleft, bottomleft) - slopeXY(topright, bottomright)) < SLOPE_ERROR_HEIGHT) << " "
                  << (std::abs(slopeYX(topleft, topright) - slopeYX(bottomleft, bottomright)) < SLOPE_ERROR_WIDTH) << " "
                  << (real_diag >= MIN_DIAG) << " "
                  << (real_diag <= MAX_DIAG) << " "
                  << validBoxColor() << std::endl;
    }

    // See if the diag is about the right length, if the width and height are about right,
    // and if a circle in the center of the possible box is almost entirely black.
    if (h >= approx_height-HEIGHT_ERROR && h <= approx_height+HEIGHT_ERROR &&
        std::abs(distance(topleft, bottomright) - distance(topright, bottomleft)) < DIAG_ERROR && // Diagonals same
        std::abs(distance(topleft, bottomleft) - distance(topright, bottomright)) < DIAG_ERROR && // Height same
        std::abs(distance(topleft, topright) - distance(bottomleft, bottomright)) < DIAG_ERROR && // Width same
        std::abs(slopeXY(topleft, bottomleft) - slopeXY(topright, bottomright)) < SLOPE_ERROR_HEIGHT && // Height slope
        std::abs(slopeYX(topleft, topright) - slopeYX(bottomleft, bottomright)) < SLOPE_ERROR_WIDTH  && // Width slope
        real_diag >= MIN_DIAG && real_diag <= MAX_DIAG && // Get rid of 1-5px boxes
        validBoxColor()) // Black enough in box and white enough around box
    {
        img.mark(topleft);
        img.mark(topright);
        img.mark(bottomleft);
        img.mark(bottomright);

        return true;
    }

    return false;   
}

// Find mid point
Coord Box::midPoint(const Coord& p1, const Coord& p2) const
{
    return Coord((p1.x+p2.x)/2, (p1.y+p2.y)/2);
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
            if (y >= lineFunctionY(topleft,    topright,    x) &&
                y <= lineFunctionY(bottomleft, bottomright, x) &&
                x >= lineFunctionX(topleft,    bottomleft,  y) &&
                x <= lineFunctionX(topright,   bottomright, y))
            {
                if (img.black(Coord(x,y)))
                // TODO: which of these is better?
                //if (blobs.label(Coord(x,y)) == label)
                    ++inside_black;

                ++inside_total;
            }
            // In area around box
            else if (y >= lineFunctionY(tl, tr, x) &&
                 y <= lineFunctionY(bl, br, x) &&
                 x >= lineFunctionX(tl, bl, y) &&
                 x <= lineFunctionX(tr, br, y))
            {
                if (img.black(Coord(x,y)))
                //if (blobs.label(Coord(x,y)) == label)
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

// If we've been to pixel before, go back till we can go some place new.
EdgePair Box::findEdge(const Coord& p, const std::vector<Coord>& path) const
{
    typedef std::vector<Coord>::size_type size_type;
    
    int index = -1;

    // Start at this point
    Coord position = p;

    // If that doesn't work, start at the last pixel
    size_type path_index = (path.size()>0)?(path.size()-1):0;

    // Keep going backwards until we find one with another option
    while (true)
    {
        index = findIndex(position, path);

        // We found an option!
        if (index != -1)
            break;

        // We ran out of options
        if (path_index == 0)
            break;
        
        // Go back one pixel
        position = path[path_index];
        --path_index;
    }

    return EdgePair(position, index);
}

// Loop through the matrix looking for the first black pixel with a white (not
// part of this object) pixel previous to it that we haven't been to. Return -1
// if there's no options (e.g. white pixel in middle of black ring).
int Box::findIndex(const Coord& p, const std::vector<Coord>& path, bool check_path) const
{
    typedef std::array<Coord, 8>::size_type size_type;
    
    int result = -1;

    for (size_type i = 0; i < matrix.size(); ++i)
    {
        // Back one pixel with looping, i.e. 0-1 = 7
        const int back = (i==0)?(matrix.size()-1):(i-1);

        const Coord current  = p+matrix[i];
        const Coord previous = p+matrix[back];

        if (blobs.label(current) == label && blobs.label(previous) != label &&
            (!check_path || std::find(path.rbegin(), path.rend(), previous) == path.rend()))
        {
            result = back;
            break;
        }
    }

    return result;
}
