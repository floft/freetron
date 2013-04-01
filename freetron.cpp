/*
 * Freetron - an open-source software scantron implementation
 *
 * Todo:
 *   - Improve findBlack()
 *   - Improve threshold()
 *   - When a box is missing, calculate supposed position
 *   - Don't crash on corrupt or non-supported PDFs
 *   - Specify max memory usage, number of threads, debug mode, quiet, etc.
 *   - Would multithreading extract() increase speed?
 */

#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <IL/il.h>
#include <podofo/podofo.h>

#include "box.h"
#include "log.h"
#include "read.h"
#include "data.h"
#include "boxes.h"
#include "rotate.h"
#include "pixels.h"
#include "extract.h"
#include "options.h"
#include "threading.h"

enum class Args
{
    Unknown,
    Help,
    ID
};

void help()
{
    std::cerr << "Usage: freetron -i TeacherID in.pdf" << std::endl;
}

void invalid()
{
    std::cerr << "Invalid argument (see \"-h\" for usage)" << std::endl;
    std::exit(1);
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

        // Determine what is black (changes in BW, color, and grayscale)
        double black = findBlack(*image, blobs, boxes, data);

        // Find ID number
        int id = findID(*image, blobs, boxes, data, black);

        std::vector<Answer> answers;

        // Don't bother finding answers if we couldn't even get the student ID
        if (id != DefaultID)
            answers = findAnswers(*image, blobs, boxes, data, black);

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

    // Argument parsing
    std::string filename;
    int teacher = DefaultID;

    std::map<std::string, Args> options = {{
        { "-h",      Args::Help },
        { "--help",  Args::Help },
        { "-i",      Args::ID },
        { "--id",    Args::ID }
    }};

    for (int i = 1; i < argc; ++i)
    {
        switch (options[argv[i]])
        {
            case Args::Help:
                help();
                return 1;
            case Args::ID:
                ++i;

                if (i == argc)
                    invalid();

                try
                {
                    teacher = std::stoi(argv[i]);
                }
                catch (const std::invalid_argument& e)
                {
                    std::cerr << "Error: invalid teacher ID" << std::endl;
                }
                break;
            default:
                if (!filename.empty())
                    invalid();

                filename = argv[i];
                break;
        }
    }

    if (teacher == DefaultID)
    {
        std::cerr << "Error: teacher ID cannot be the default ID" << std::endl;
        return 1;
    }

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
    std::cout << std::left;

    if (DEBUG)
        std::cout << std::setw(5)  << "#";

    std::cout << std::setw(10) << "ID"
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
                if (DEBUG)
                    std::cout << std::setw(5) << i.thread_id;

                std::cout << std::setw(10) << i.id << "\t";

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
        typedef std::vector<Answer>::size_type size_type;

        // Grade student's exams
        std::map<int, double> scores;
        
        for (const Info& i : results)
        {
            if (i.id == DefaultID)
            {
                std::cerr << i.thread_id << ": failed to determine student ID" << std::endl;
            }
            else if (i.id != teacher)
            {
                if (DEBUG)
                    std::cout << std::left << std::setw(5) << i.thread_id;

                std::cout << std::left << std::setw(10) << i.id << "\t";

                int same = 0;

                // Only score the ones that the teacher didn't leave blank
                for (size_type q = 0; q < i.answers.size(); ++q)
                {
                    if (key[q] != Answer::Blank)
                    {
                        std::cout << i.answers[q] << " ";

                        if (key[q] == i.answers[q])
                            ++same;
                    }
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
        std::cerr << "Key not found. Given IDs:" << std::endl;

        for (const Info& i : results)
            if (i.id != DefaultID)
                std::cerr << "  " << i.id << std::endl;
    }
    
    return 0;
}
