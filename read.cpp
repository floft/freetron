#include "read.h"

// Percentage of pixels in a circle that are a certain label
// 0 = no label, 1 = all label
double percentageLabel(const Pixels& img, const Blobs& blobs, const Bubble& b)
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

    int black  = 0;
    int total  = 0;

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

// Find ID number from card
int findID(Pixels& img, const Blobs& blobs,
    const std::vector<Coord>& boxes, const Data& data)
{
    typedef std::vector<Coord>::size_type size_type;

    int id = 0;
    std::map<int, int> filled;
    
    const size_type& start_box = ID_START;
    const size_type& end_box   = ID_END;

    // If the boxes don't exist or the boxes are skewed give up
    if (boxes.size() < end_box || !vertical(boxes, start_box, end_box))
        return 0;

    // Calculate relative values, use ID height to be more accurate
    const int box_height = data.width/ASPECT;
    const int id_height  = boxes[end_box-1].y - boxes[start_box-1].y;
    const int id_width   = 1.0*ID_WIDTH/ID_HEIGHT*id_height;

    // Get all the bubbles (first point of an object) within this ID box. Extend
    // it a bit just to make sure we get everything.
    const std::vector<Bubble> bubbles = findBubbles(img, blobs, data.diag,
         Coord(boxes[BOT_START].x - box_height, boxes[start_box-1].y - box_height),
         Coord(boxes[BOT_START].x + id_width + box_height, boxes[end_box-1].y + box_height));

    // Determine which of these bubbles are filled
    for (size_type i = start_box-1; i < end_box && i < boxes.size(); ++i)
    {
        for (const Bubble& b : bubbles)
        {
            // Look at bubbles in this row
            if (std::abs(boxes[i].y - b.coord.y) < VERT_STRAY &&
                percentageLabel(img, blobs, b) > MIN_BLACK)
            {
                if (DEBUG)
                    img.mark(b.coord);

                // at x = position, the value is box # - start_box + 1 (0 = start_box)
                filled[b.coord.x] = i - start_box + 1;
            }
        }
    }

    // Get ID number from map
    int i;
    std::map<int, int>::const_iterator iter;

    for (i = filled.size()-1, iter = filled.begin(); iter != filled.end(); ++iter, --i)
    {
        // Add with zeros for place value
        id += (iter->second)*pow(10, i);
    }

    return id;
}

std::vector<Answer> findAnswers(Pixels& img, const Blobs& blobs,
    const std::vector<Coord>& boxes, const Data& data)
{
    std::vector<Answer> answers(Q_TOTAL);

    int column = 0;
    int box = Q_START - 1;

    // The amount to jump is half the distance between second two boxes
    // on the bottom row
    const int jump = 0.5*(boxes[BOT_START+1].x - boxes[BOT_START].x);

    // Same as before
    const int box_height = data.width/ASPECT;

    // The width of one column
    const int question_width = boxes[BOT_START+2].x - boxes[BOT_START].x + 2*box_height;

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
             Coord(start - box_height, boxes[box].y - box_height),
             Coord(end   + box_height, boxes[box].y + box_height));

        for (const Bubble& b : bubbles)
        {
            if (percentageLabel(img, blobs, b) > MIN_BLACK)
            {
                if (DEBUG)
                    img.mark(b.coord);

                // Note the integer division
                // TODO: is this really the best way to do this?
                answers[q] = (Answer)(5*(b.coord.x - start + box_height)/question_width + 1);
            }
        }

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

std::vector<Bubble> findBubbles(const Pixels& img, const Blobs& blobs, const int diag,
    const Coord& p1, const Coord& p2)
{
    std::vector<Bubble> bubbles;
    const std::vector<Coord> local_blobs = blobs.in(p1, p2);

    for (const Coord& object : local_blobs)
    {
        const Outline outline(img, blobs, object, MAX_ITERATIONS);
        const Coord center = findCenter(outline.points());
        const Coord p1 = farthestFromPoint(center, outline.points());
        const Coord p2 = farthestFromPoint(p1,     outline.points());
        const double d = distance(p1, p2);

        if (d > MIN_DIAG && d < MAX_DIAG &&  // Decent size
            d > diag - DIAG_ERROR) // Have a lower bound on the diagonal
        {
            // w / (w/h) = h, h/2 = "radius" of the bubble
            bubbles.push_back(Bubble(d/BUBBLE_ASPECT/2, blobs.label(object), center));
        }
    }

    return bubbles;
}
