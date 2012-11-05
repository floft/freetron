#include <vector>
#include <iostream>
#include <stdexcept>
#include <Magick++.h>

#include "options.h"
#include "rotate.h"
#include "read.h"

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
	pdf.monochrome(1);
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
		cout << error.what() << endl;
		return 1;
	}
	
	// Need this to be an image
	pdf.magick("png");

	// Find top boxes
	Pixels view(pdf);
	unsigned int width  = pdf.columns();
	unsigned int height = pdf.rows();

	// Find boxes on left of card
	unsigned int box_width;
	vector< vector<unsigned int> > boxes = findBoxes(view, width, height, box_width, pdf);

	// Find rotation needed to make boxes vertical
	unsigned int x, y;
	double rotation = findRotation(view, boxes, x, y, width, height, pdf);

	if (rotation != 0)
	{
		// Rotate
		Image large(Geometry(width*2, height*2), Color("white"));
		large.density(Geometry(300, 300));
		large.draw(DrawableCompositeImage(width-x, height-y, pdf));
		large.rotate(rotation*180.0/pi); //Clockwise is -deg
		large.trim();
		
		// Add border
		//large.draw(DrawableTranslation(-20, -20));
		pdf = large;
	}

	pdf.write("cow.orig.png");
	
	// Find boxes on left of card again since it was rotated
	Pixels alignedView(pdf);
	width  = pdf.columns();
	height = pdf.rows();
	boxes  = findRealBoxes(alignedView, width, height, box_width, pdf);

	// Find ID number
	unsigned int id = findID(alignedView, boxes, width, height, box_width, pdf);

	// Print it
	cout << id << endl;

	pdf.write("cow.png");

	return 0;
}
