/*
 * Freetron - an open-source software scantron implementation
 *
 * Todo:
 *   - Use some library other than graphicsmagick for pixel access
 *     and rotation
 *   - Use Threading class for each image
 */

#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <Magick++.h>
#include <podofo/podofo.h>

#include "extract.h"
#include "options.h"
#include "rotate.h"
#include "data.h"
#include "read.h"
#include "boxes.h"

using namespace std;
using namespace Magick;
using namespace PoDoFo;

void help()
{
	cout << "Usage: freetron in.pdf" << endl;
}

int main(int argc, char* argv[])
{
	InitializeMagick(argv[0]);
	vector<Image> images;

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
	catch (Exception &error)
	{
		cerr << error.what() << endl;
		return 1;
	}
	catch (const PdfError& error)
	{
		error.PrintErrorMsg();
		return error.GetError();
	}
	
	// Support multi-page PDFs
	for (unsigned int i = 0; i < images.size(); ++i)
	{
		Image& image = images[i];

		// Rotate the image
		Pixels original(image);
		Coord rotate_point;
		unsigned int width  = image.columns();
		unsigned int height = image.rows();
		double rotation = findRotation(original, rotate_point, width, height);

		if (rotation != 0)
		{
			image.draw(DrawableTranslation(-rotate_point.x, -rotate_point.y));
			image.rotate(rotation*180.0/pi);
			image.trim();
		}

		// Find all the boxes on the left, and find box_height while we're at it
		Pixels rotated(image);
		width  = image.columns();
		height = image.rows();
		unsigned int box_width;
		vector<Coord> boxes = findBoxes(rotated, width, height, box_width);

		// Find ID number
		unsigned int id = findID(rotated, boxes, width, height, box_width, image);

		// Debug information
		if (DEBUG)
		{
			for (unsigned int j = 0; j < boxes.size(); ++j)
			{
				image.fillColor("orange");
				image.draw(DrawableRectangle(boxes[j].x-5, boxes[j].y-5,
					boxes[j].x+5, boxes[j].y+5));
			}
			
			stringstream s;
			s << "debug" << i << ".png";
			image.write(s.str());
		}

		// For now just print it. Later we'll do stuff with it.
		if (id > 0)
			cout << id << endl;
		else
			cout << "Could not determine ID" << endl;
	}
	
	return 0;
}
