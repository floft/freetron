/*
 * Freetron - an open-source software scantron implementation
 *
 * Todo:
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
        const Blobs blobs(*image);

        // Box information for this image
        Data data;
        
        // Find all the boxes
        std::vector<Coord> boxes = findBoxes(*image, blobs, data);

        if (boxes.size() > TOTAL_BOXES)
            throw std::runtime_error("too many boxes detected");

        if (boxes.size() < TOTAL_BOXES)
            throw std::runtime_error("some boxes not detected");

        // Rotate the image
        Coord rotate_point;
        double rotation = findRotation(*image, boxes, rotate_point);

        // Negative since the origin is the top-left point (this is the 4th quadrant)
        if (rotation != 0)
        {
            image->rotate(-rotation, rotate_point);
            image->rotateVector(boxes, rotate_point, -rotation);
        }

        // Find ID number
        int id = findID(*image, blobs, boxes, data);

        std::vector<Answer> answers;

        // Don't bother finding answers if we couldn't even get the student ID
        if (id > 0)
            answers = findAnswers(*image, blobs, boxes, data);

        // Debug information
        if (DEBUG)
        {
            for (const Coord& box : boxes)
                image->mark(box);
            
            std::ostringstream s;
            s << "debug" << thread_id << ".png";
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

    if (teacher == 0)
    {
        log("teacher ID cannot be 0");
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
    std::cout << "Exam Results (key first)" << std::endl;

    int total = 0;
    bool found = false;
    std::vector<Answer> key;

    for (const Info& i : results)
    {
        if (i.id == teacher)
        {
            if (found)
                log("found multiple keys, using last one");

            std::cout << i.thread_id << ": " << std::setw(10) << i.id << " -- ";

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
        }
    }

    // Grade student's exams
    std::map<int, double> scores;
    
    for (const Info& i : results)
    {
        if (i.id > 0)
        {
            if (i.id == teacher)
                continue;

            std::cout << i.thread_id << ": " << std::setw(10) << i.id << " -- ";

            int same = 0;

            for (int q = 0; q < total; ++q)
            {
                std::cout << i.answers[q] << " ";

                if (key[q] == i.answers[q])
                    ++same;
            }

            std::cout << std::endl;

            scores[i.id] = 1.0*same/total;
        }
        else
        {
            std::cout << i.thread_id << ": failed to determine student ID" << std::endl;
        }
    }
    
    std::cout << std::endl;

    // Output scores
    std::cout << "Scores" << std::endl;

    for (const std::pair<int, double>& score: scores)
    {
        std::cout << std::setw(10) << score.first << ": " << score.second*100 << "%" << std::endl;
    }
    
    return 0;
}
