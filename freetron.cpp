/*
 * Freetron - an open-source software scantron implementation
 *
 * Todo:
 *   - Fix skipping boxes that start with a pixel and one down and to the left
 *   - Rewrite student ID detection code to check columns and use auto-
 *     decreasing sensitivity
 *   - Detect "answer black" before doing any of the detection algorithms
 *
 *   - Take amount of memory into consideration when creating threads
 *   - When a box is missing (box #26 on cat22.pdf), calculate supposed position
 *   - Make image extraction multi-threaded for computing isBlack bool or maybe
 *      start processing other pages after key has been processed while reading
 *      other images
 */

#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <IL/il.h>
#include <podofo/podofo.h>

#include "extract.h"
#include "pixels.h"
#include "options.h"
#include "rotate.h"
#include "data.h"
#include "read.h"
#include "boxes.h"
#include "box.h"
#include "threading.h"
#include "log.h"

void help()
{
    std::cout << "Usage: freetron in.pdf TeacherID" << std::endl;
}

// Return type for threads
struct Info
{
    int thread_id = 0;
    int id = 0;
    std::vector<Answer> answers;

    Info() { }
    Info(int t) :thread_id(t) { }
    Info(int t, int i, const std::vector<Answer>& answers)
        :thread_id(t), id(i), answers(answers) { }
};

// Called in a new thread for each image
Info parseImage(Pixels* image)
{
    // Use this to get a unique ID each time this function is called,
    // used for writing out the debug images
    static int static_thread_id = 0;
    const int thread_id = static_thread_id++;

    // When this thread has an error, write message including thread id, but
    // continue processing the rest of the images.
    try
    {
        // Find all blobs in the image
        Blobs blobs(*image);

        // Box information for this image
        Data data;
        
        // Find all the boxes
        std::vector<Coord> boxes = findBoxes(*image, blobs, data);

        if (boxes.size() > TOTAL_BOXES)
            throw std::runtime_error("too many boxes detected");

        if (boxes.size() < TOTAL_BOXES)
            throw std::runtime_error("some boxes not detected");

        if (DEBUG)
        {
            std::ostringstream s_orig;
            s_orig << "debug" << thread_id << "_orig.png";
            image->save(s_orig.str(), true, false, true);
        }

        // Rotate the image
        Coord rotate_point;
        double rotation = findRotation(*image, boxes, rotate_point);

        // Negative since the origin is the top-left point (this is the 4th quadrant)
        if (rotation != 0)
        {
            image->rotate(-rotation, rotate_point);
            image->rotateVector(boxes, rotate_point, -rotation);

            // The blobs are constant, so just recalculate them all
            blobs = Blobs(*image);
        }

        // Sort all boxes if with respect to y if we rotated counter-clockwise
        if (rotation > 0)
            std::sort(boxes.begin(), boxes.end());

        // Sort the bottom row of boxes by increasing x coordinates.
        std::sort(boxes.begin()+BOT_START-1, boxes.begin()+BOT_END, CoordXSort());

        // Find ID number
        int id = findID(*image, blobs, boxes, data);

        std::vector<Answer> answers;

        // Don't bother finding answers if we couldn't even get the student ID
        if (id != DefaultID)
            answers = findAnswers(*image, blobs, boxes, data);

        // Debug information
        if (DEBUG)
        {
            for (const Coord& box : boxes)
                image->mark(box);
            
            std::ostringstream s;
            s      << "debug" << thread_id << ".png";
            image->save(s.str());
        }
    
        return Info(thread_id, id, answers);
    }
    catch (const std::runtime_error& error)
    {
        if (DEBUG)
        {
            std::ostringstream s;
            s << "debug" << thread_id << "_error.png";
            image->save(s.str());
        }

        std::ostringstream msg;
        msg << "thread #" << thread_id << ", " << image->filename() << " - " << error.what();
        log(msg.str());
    }

    return Info(thread_id);
}

int main(int argc, char* argv[])
{
    ilInit();
    std::vector<Pixels> images;

    // Simple help message
    if (argc != 3 || (argc >= 2 &&
       (std::strcmp(argv[1], "-h") == 0 ||
        std::strcmp(argv[1], "--help") == 0)))
    {
        help();
        return 1;
    }

    std::string filename = argv[1];
    int teacher = std::stoi(argv[2]);

    try
    {
        // Get the images from the PDF
        images = extract(filename);
    }
    catch (const std::runtime_error& error)
    {
        log(filename + ", " + error.what());
        return 1;
    }
    catch (const PoDoFo::PdfError& error)
    {
        error.PrintErrorMsg();

        // Don't write this to screen
        log(filename + ", " + error.what(), LogType::Error, false);

        return error.GetError();
    }
    
    // Find ID of each page in separate thread
    std::vector<Info> results = threadForEach(images, parseImage);

    // Find the key based on the teacher's ID
    std::cout << std::left
              << std::setw(5)  << "#"
              << std::setw(10) << "ID"
              << "\tAnswers (key first)"
              << std::endl;

    int total = 0;
    bool found = false;
    std::vector<Answer> key;

    for (const Info& i : results)
    {
        if (i.id == teacher)
        {
            if (found)
            {
                log("key already found, skipping this one");
            }
            else
            {
                std::cout << std::setw(5) << i.thread_id
                          << std::setw(10) << i.id << "\t";

                for (const Answer b : i.answers)
                {
                    if (b != Answer::Blank)
                    {
                        std::cout << b << " ";
                        ++total;
                    }
                }

                std::cout << std::endl;

                key = i.answers;
                found = true;
            }
        }
    }

    if (found)
    {
        // Grade student's exams
        std::map<int, double> scores;
        
        for (const Info& i : results)
        {
            if (i.id == DefaultID)
            {
                std::cout << i.thread_id << ": failed to determine student ID" << std::endl;
            }
            else if (i.id != teacher)
            {
                std::cout << std::left << std::setw(5) << i.thread_id
                          << std::left << std::setw(10) << i.id << "\t";

                int same = 0;

                for (int q = 0; q < total; ++q)
                {
                    std::cout << i.answers[q] << " ";

                    if (key[q] == i.answers[q])
                        ++same;
                }

                std::cout << std::endl;

                scores[i.id] = (total>0)?1.0*same/total:1;
            }
        }
        
        // Output scores
        std::cout << std::endl << "Scores" << std::endl;

        for (const std::pair<int, double>& score: scores)
        {
            std::cout << "  " << std::left << std::setw(10) << score.first << " "
                      << std::fixed << std::setprecision(2) << std::right << std::setw(6)
                      << score.second*100 << "%" << std::endl;
        }
    }
    else
    {
        log("key not found");
    }
    
    return 0;
}
