#include "read.h"

// Percentage of pixels in the bubble that are a certain label
// 0 = no label, 1 = all label
// How black the bubble is. 0 = none this label, 1 = all this label
double bubbleBlackness(const Pixels& img, const Blobs& blobs, const Bubble& b)
{
    // Find square around circle of radius r centered at (x,y)
    Square s(img, b.coord.x, b.coord.y, b.radius);
    const int x1    = s.topLeft().x;
    const int y1    = s.topLeft().y;
    const int x2    = s.bottomRight().x;
    const int y2    = s.bottomRight().y;
    const int mid_x = s.midPoint().x;
    const int mid_y = s.midPoint().y;

    // Maybe this makes it a bit faster
    const int r2 = b.radius*b.radius;

    int black = 0;
    int total = 0;

    for (int search_y = y1; search_y < y2; ++search_y)
    {
        for (int search_x = x1; search_x < x2; ++search_x)
        {
            if (std::pow(std::abs(search_x-mid_x),2) + std::pow(std::abs(search_y-mid_y),2) <= r2)
            {
                if (blobs.label(Coord(search_x, search_y)) == b.label)
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

// See if the boxes are skewed
bool vertical(const std::vector<Coord>& boxes,
    const int start_box, const std::vector<Coord>::size_type end_box)
{
    typedef std::vector<Coord>::size_type size_type;

    // (start_box-1)+1 = start_box
    for (size_type i = start_box; i < boxes.size() && i < end_box; ++i)
        if (boxes[i].x > boxes[i-1].x+MAX_ERROR || boxes[i].x < boxes[i-1].x-MAX_ERROR)
            return false;
    
    return true;
}

int findID(Pixels& img, const Blobs& blobs,
    const std::vector<Coord>& boxes, const Data& data,
    const double min_black)
{
    typedef std::vector<Coord>::size_type size_type;

    int id = DefaultID;
    std::map<int, int> filled;
    
    // Get rid of compilation warnings in vertical()
    const size_type& start_box = ID_START;
    const size_type& end_box   = ID_END;

    // If the boxes don't exist or the boxes are skewed give up
    if (boxes.size() < end_box || !vertical(boxes, start_box, end_box))
        return DefaultID;

    const double jump = 0.5*(boxes[BOT_START+1].x - boxes[BOT_START].x);
    const double half_jump = jump/2;
    const int vert_jump = (boxes[start_box].y-boxes[start_box-1].y);
    const int y_start = boxes[start_box-1].y;
    const int y_end   = boxes[end_box-1].y;

    std::vector<int> digits(ID_LENGTH);

    // The ten-digit number
    for (int i = 0; i < ID_LENGTH; ++i)
    {
        // Get bubbles in this column of the number
        const int center = boxes[BOT_START].x + jump*i;
        const std::vector<Bubble> bubbles = findBubbles(img, blobs, data.diag,
             Coord(center - half_jump, y_start),
             Coord(center + half_jump, y_end));

        // Find which bubble is filled. 9 is because there's 0-9
        const int filled = findFilled(img, blobs, bubbles, y_start, vert_jump,
            9, min_black, false);

        if (filled != DefaultFilled)
            digits.push_back(filled);
    }
        
    // Convert to number
    typedef std::vector<int>::reverse_iterator iterator;
    id = 0;
    int i = 0;

    for (iterator iter = digits.rbegin(); iter != digits.rend(); ++iter, ++i)
        id += *iter*std::pow(10, i);

    return id;
}

std::vector<Answer> findAnswers(Pixels& img, const Blobs& blobs,
    const std::vector<Coord>& boxes, const Data& data, const double min_black)
{
    std::vector<Answer> answers(Q_TOTAL);

    int column = 0;
    int box = Q_START - 1;

    // The amount to jump is half the distance between second two boxes
    // on the bottom row
    const double jump = 0.5*(boxes[BOT_START+1].x - boxes[BOT_START].x);
    
    // We need to know about how big these bubbles are
    int box_height = data.width/ASPECT;

    for (int q = 0; q < Q_TOTAL; ++q)
    {
        // X coordinates
        int start = 0;
        int end   = 0;

        // These are all relative to the bottom boxes. This would be different for every
        // type of form, but for all from this manufacturer they are the same. (Or so their
        // website seems to indicate.)
        switch (column)
        {
            case 0:
                start = boxes[BOT_START].x;
                end   = boxes[BOT_START+2].x;
                break;
            case 1:
                start = boxes[BOT_START+3].x - jump;
                end   = boxes[BOT_START+4].x + jump;
                break;
            case 2:
                start = boxes[BOT_START+5].x;
                end   = boxes[BOT_START+7].x;
                break;
            default:
                log("too many columns");
                break;
        }

        // Get all the bubbles (first point of an object) within this ID box. Extend
        // it a bit just to make sure we get everything.
        const std::vector<Bubble> bubbles = findBubbles(img, blobs, data.diag,
             Coord(start, boxes[box].y - box_height),
             Coord(end,   boxes[box].y + box_height));

        // Find which bubble is filled
        const int filled = findFilled(img, blobs, bubbles, start, jump,
            Q_OPTIONS, min_black, true);

        if (filled != DefaultFilled)
            answers[q] = (Answer)(filled+1);

        ++box;

        // For the next question look at the next column
        if (box == Q_END)
        {
            ++column;
            box = Q_START - 1;
        }
    }

    return answers;
}

std::vector<Bubble> findBubbles(Pixels& img, const Blobs& blobs, const int diag,
    const Coord& a, const Coord& b)
{
    std::vector<Bubble> bubbles;
    const std::vector<Coord> local_blobs = blobs.in(a, b);

    /*if (DEBUG)
    {
        img.line(a, Coord(b.x, a.y));
        img.line(Coord(b.x, a.y), b);
        img.line(Coord(a.x, b.y), b);
        img.line(a, Coord(a.x, b.y));
    }*/

    for (const Coord& object : local_blobs)
    {
        const Outline outline(img, blobs, object, MAX_ITERATIONS);
        const Coord center = findCenter(outline.points());

        if (center == default_coord)
            continue;

        const Coord p1 = farthestFromPoint(center, outline.points());
        const Coord p2 = farthestFromPoint(p1,     outline.points());
        const double d = distance(p1, p2);

        if (d > MIN_DIAG && d < MAX_DIAG &&  // Decent size
            d > diag - DIAG_ERROR) // Have a lower bound on the diagonal
        {
            // A circle with radius d/2 should encompass all of a bubble
            bubbles.push_back(Bubble(d/2, blobs.label(object), center));

            if (DEBUG)
            {
                for (const Coord& c : outline.points())
                    img.mark(c, 1);
            }
        }
    }

    return bubbles;
}

// use_x = true means use X coordinate, false means use Y coordinate
int findFilled(Pixels& img, const Blobs& blobs, const std::vector<Bubble>& bubbles,
    const int start, const double jump, const int options, double black, const bool use_x)
{
    Coord coord;
    int count = 0;
    double half_jump = jump/2;

    // Determine a black level that will find exactly one bubble
    do
    {
        count = 0;

        for (const Bubble& b : bubbles)
        {
            if (bubbleBlackness(img, blobs, b) > black)
            {
                ++count;
                coord = b.coord;
            }
        }

        black += 0.05;
    } while (black < 1 && count > 1);

    // If we found the bubble
    if (count > 0)
    {
        if (DEBUG)
            img.mark(coord);

        for (int i = 0; i <= options; ++i)
        {
            // Horizontal
            if (use_x)
            {
                if (coord.x < start+jump*i+half_jump)
                    return i;
            }
            // Vertical
            else
            {
                if (coord.y < start+jump*i+half_jump)
                    return i;
            }
        }
    }

    return DefaultFilled;
}

double findBlack(Pixels& img, const Blobs& blobs, const std::vector<Coord>& boxes,
    const Data& data)
{
    const double jump = 0.5*(boxes[BOT_START+1].x - boxes[BOT_START].x);
    const std::vector<Bubble> bubbles = findBubbles(img, blobs, data.diag,
        Coord(boxes[BOT_START].x, boxes[ID_START-1].y),
        Coord(boxes[BOT_START].x + jump*(ID_LENGTH-1), boxes[ID_END-1].y));
    std::vector<double> maxes;
    std::vector<double> color(bubbles.size());

    for (const Bubble& b : bubbles)
        color.push_back(bubbleBlackness(img, blobs, b));
    
    // Throw out the max value ID_LENGTH times
    for (int i = 0; i < ID_LENGTH; ++i)
    {
        std::vector<double>::iterator max = std::max_element(color.begin(), color.end());

        if (max != color.end())
        {
            maxes.push_back(*max);
            color.erase(max);
        }
    }

    double avg     = average(color);
    double max_avg = average(maxes);

    // I'm guessing that the halfway point between the filled-in bubbles' average
    // and the not-filled-in bubbles' average is a decent black value
    //
    // TODO: fix this algorithm
    //  - Don't throw out all 10 since the ID number might not be completely filled in
    //  - Is d/2 the correct radius to search for?
    double black = (avg+max_avg)/2;

    if (black == 0)
        std::cout << "black is 0" << std::endl;

    return (black>0)?black:MIN_BLACK;
}
