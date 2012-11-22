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
	pdf.magick("png");

	// Rotate the image
	Pixels original(pdf);
	unsigned int x, y;
	unsigned int width  = pdf.columns();
	unsigned int height = pdf.rows();
	double rotation = findRotation(original, x, y, width, height);

	if (rotation != 0)
	{
		pdf.draw(DrawableTranslation(-x, -y));
		pdf.rotate(rotation*180.0/pi);
		pdf.trim();
	}

	// Find all the boxes on the left
	Pixels rotated(pdf);
	width  = pdf.columns();
	height = pdf.rows();
	vector< vector<Coord> > boxes = findBoxes(rotated, width, height, pdf);

	for (unsigned int i = 0; i < boxes.size(); ++i)
	{
		pdf.fillColor("green");
		pdf.draw(DrawableRectangle(boxes[i][0].x, boxes[i][0].y, boxes[i][0].x+2, boxes[i][0].y+2));
		pdf.fillColor("orange");
		pdf.draw(DrawableRectangle(boxes[i][1].x, boxes[i][1].y, boxes[i][1].x+2, boxes[i][1].y+2));
	}

	pdf.write("cow.png");
	
	return 0;
}
