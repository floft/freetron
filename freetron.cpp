/*
 * Freetron - an open-source software scantron implementation
 *
 * Todo:
 *   - Automatically determine BOX_WIDTH, BOX_HEIGHT, DIAGONAL, MIN_BLACK, FIRST_JUMP, ...
 *   - Support multi-page PDFs
 */

#include <vector>
#include <iostream>
#include <stdexcept>
#include <Magick++.h>

#include "options.h"
#include "rotate.h"
#include "data.h"
#include "read.h"
#include "boxes.h"

using namespace std;
using namespace Magick;

void help()
{
	cout << "Usage: freetron in.pdf" << endl;
}

int main(int argc, char* argv[])
{
	InitializeMagick(argv[0]);
	Image pdf;
	pdf.density(Geometry(300,300));
	pdf.backgroundColor(Color("white"));

	if (argc != 2)
	{
		help();
		return 1;
	}

	// Attempt to get the PDF
	try
	{
		pdf.read(argv[1]);
	}
	catch (Exception &error)
	{
		cerr << error.what() << endl;
		return 1;
	}
	
	// Need this to be an image
	// TODO: Use tiff for multi-page PDFs?
	pdf.magick("png");

	// Rotate the image
	Pixels original(pdf);
	Coord rotate_point;
	unsigned int width  = pdf.columns();
	unsigned int height = pdf.rows();
	double rotation = findRotation(original, rotate_point, width, height);

	/*if (rotation != 0)
	{
		pdf.draw(DrawableTranslation(-rotate_point.x, -rotate_point.y));
		pdf.rotate(rotation*180.0/pi);
		pdf.trim();
	}

	// Find all the boxes on the left, and find box_height while we're at it
	Pixels rotated(pdf);
	width  = pdf.columns();
	height = pdf.rows();
	unsigned int box_height;
	vector<Coord> boxes = findBoxes(rotated, width, height, box_height);

	// Find ID number
	unsigned int id = findID(rotated, boxes, width, height, box_height, pdf);

	// Debug information
	if (DEBUG)
	{
		for (unsigned int i = 0; i < boxes.size(); ++i)
		{
			pdf.fillColor("orange");
			pdf.draw(DrawableRectangle(boxes[i].x-5, boxes[i].y-5,
				boxes[i].x+5, boxes[i].y+5));
		}
		
		pdf.write("debug.png");
	}

	// For now just print it. Later we'll do stuff with it.
	cout << id << endl;*/

	pdf.write("debug.png");
	
	return 0;
}
