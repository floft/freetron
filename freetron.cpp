/*
 * Freetron - an open-source software scantron implementation
 *
 * Todo:
 *   - Add << and >> for Pixels for easy testing
 *   - Develop better algorithm for finding if bubble is filled in
 *   - Use Threading class for each image
 *   - Use size_t, iterators, etc. instead of converting all to uint and whatnot
 *   - Use dynamic_bitset for storing bools in Pixels
 *   - Write [multithreaded?] rotation code
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
#include "threading.h"

using namespace std;
using namespace PoDoFo;

void help()
{
	cout << "Usage: freetron in.pdf" << endl;
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
Info parseImage(Pixels& image, unsigned int thread_id)
{
	// Rotate the image
	Coord rotate_point;
	double rotation = findRotation(image, rotate_point, image.width(), image.height());

	// Negative since the origin is the top-left point
	if (rotation != 0)
		image.rotate(-rotation, rotate_point);

	// Find all the boxes on the left, and find box_height while we're at it
	unsigned int box_width;
	vector<Coord> boxes = findBoxes(image, image.width(), image.height(), box_width);

	// Find ID number
	unsigned int id = findID(image, boxes, image.width(), image.height(), box_width);

	// Debug information
	if (DEBUG)
	{
		for (const Coord& box : boxes)
			image.mark(box);
		
		ostringstream s;
		s << "debug" << thread_id << ".png";
		image.save(s.str());
	}

	return Info(thread_id, id);
}

int main(int argc, char* argv[])
{
	ilInit();
	vector<Pixels> images;

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
	catch (const runtime_error& error)
	{
		cerr << "Error: " << error.what() << endl;
		return 1;
	}
	catch (const PdfError& error)
	{
		error.PrintErrorMsg();
		return error.GetError();
	}
	
	// Find ID of each page in separate thread
	Threading<Info> t(images, parseImage);
	t.run();
	vector<Info> ids = t.results();

	for (const Info& i : ids)
		cout << i.thread_id << ": " << ((i.id>0)?i.id:"error") << endl;


	//for (Pixels& image : images)
	//	parseImage(image, count);
	
	return 0;
}
