/*
 * Freetron - an open-source software scantron implementation
 *
 * Todo:
 *   - Decrease memory usage (take into consideration when creating threads?)
 *   - Fix algorithm to walk edge on box with stray pixel in cat1.pdf
 *   - Use bottom boxes to find bubbles
 *   - Pass in const Pixels& wherever possible
 *   - Develop better algorithm for finding if bubble is filled in
 *   - Make image extraction multi-threaded for computing isBlack bool or maybe
 *      start processing other pages after key has been processed while reading
 *      other images
 */

#include <vector>
#include <cstring>
#include <sstream>
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

void help()
{
	std::cout << "Usage: freetron in.pdf" << std::endl;
}

// Return type for threads
struct Info
{
	int thread_id = 0;
	int id = 0;
	double rotation = 0;

	Info() { }
	Info(int t) :thread_id(t) { }
	Info(int t, int i, double r)
		:thread_id(t), id(i), rotation(r) { }
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
		// Box information for this image
		BoxData data;
		
		// Find all the boxes
		std::vector<Coord> boxes = findBoxes(*image, &data);

		if (boxes.size() != TOTAL_BOXES)
		{
			for (const Coord& box : boxes)
				image->mark(box);
			
			std::ostringstream s;
			s << "debug" << thread_id << ".png";
			image->save(s.str());
		}

		if (boxes.size() > TOTAL_BOXES)
			throw std::runtime_error("too many boxes detected");

		if (boxes.size() < TOTAL_BOXES)
			throw std::runtime_error("some boxes not detected");

		// Rotate the image
		Coord rotate_point;
		double rotation = findRotation(*image, boxes, rotate_point);

		// Negative since the origin is the top-left point
		if (rotation != 0)
		{
			image->rotate(-rotation, rotate_point);
			image->rotateVector(boxes, rotate_point, -rotation);
		}

		// Find ID number
		int id = findID(*image, boxes, &data);

		// Debug information
		if (DEBUG)
		{
			for (const Coord& box : boxes)
				image->mark(box);
			
			std::ostringstream s;
			s << "debug" << thread_id << ".png";
			image->save(s.str());
		}
	
		return Info(thread_id, id, rotation*180/pi);
	}
	catch (const std::runtime_error& error)
	{
		if (DEBUG)
		{
			std::ostringstream s;
			s << "debug" << thread_id << "_error.png";
			image->save(s.str());
		}

		std::cerr << "Error on thread #" << thread_id << ": " << error.what() << std::endl;
	}

	return Info(thread_id);
}

int main(int argc, char* argv[])
{
	ilInit();
	std::vector<Pixels> images;
	std::vector<Info> results;

	// Simple help message
	if (argc != 2 || (argc >= 2 &&
		(std::strcmp(argv[1], "-h") == 0 ||
		 std::strcmp(argv[1], "--help") == 0)))
	{
		help();
		return 1;
	}

	try
	{
		// Get the images from the PDF
		images = extract(argv[1]);
	}
	catch (const std::runtime_error& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}
	catch (const PoDoFo::PdfError& error)
	{
		error.PrintErrorMsg();
		return error.GetError();
	}
	
	// Find ID of each page in separate thread
	results = threadForEach(images, parseImage);

	// Parse results
	for (const Info& i : results)
		if (i.id > 0)
			std::cout << i.thread_id << ": " << i.id << " " << i.rotation << std::endl;
		else
			std::cout << i.thread_id << ": error " << i.rotation << std::endl;
	
	return 0;
}
