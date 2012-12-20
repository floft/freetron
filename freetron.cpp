/*
 * Freetron - an open-source software scantron implementation
 *
 * Todo:
 *   - Add << and >> for Pixels for easy testing
 *   - Use size_t, iterators, int, etc. instead of unsigned int
 *   - Develop better algorithm for finding if bubble is filled in
 *   - Use dynamic_bitset for storing bools in Pixels
 *   - Make image extraction multi-threaded for computing isBlack bool or maybe
 *      start processing other pages after key has been processed while reading
 *      other images
 *   - Use neural network?
 */

#include <vector>
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
	unsigned int thread_id = 0;
	unsigned int id = 0;

	Info() { }
	Info(unsigned int t, unsigned int i)
		:thread_id(t), id(i) { }
};

// Called in a new thread for each image
Info parseImage(Pixels* image)
{
	// Use this to get a unique ID each time this function is called,
	// used for writing out the debug images
	static unsigned int thread_id = 0;
	++thread_id;

	// Box information for this image, needed because of multithreading
	BoxData data;

	// Rotate the image
	Coord rotate_point;
	double rotation = findRotation(*image, rotate_point, image->width(), image->height(), &data);

	// Negative since the origin is the top-left point
	if (rotation != 0)
		image->rotate(-rotation, rotate_point);

	// Find all the boxes on the left, and find box_height while we're at it
	unsigned int box_width;
	std::vector<Coord> boxes = findBoxes(*image, image->width(), image->height(), box_width, &data);

	// Find ID number
	unsigned int id = findID(*image, boxes, image->width(), image->height(), box_width);

	// Debug information
	if (DEBUG)
	{
		for (const Coord& box : boxes)
			image->mark(box);
		
		std::ostringstream s;
		s << "debug" << thread_id << ".png";
		image->save(s.str());
	}

	return Info(thread_id, id);
}

int main(int argc, char* argv[])
{
	ilInit();
	std::vector<Pixels> images;

	if (argc != 2)
	{
		help();
		return 1;
	}

	// Attempt to get the images from the PDF
	try
	{
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
	std::vector<Info> results = threadForEach(images, parseImage);

	for (const Info& i : results)
		if (i.id > 0)
			std::cout << i.thread_id << ": " << i.id << std::endl;
		else
			std::cout << i.thread_id << ": error" << std::endl;
	
	return 0;
}
