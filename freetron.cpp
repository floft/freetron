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

	Pixels view(pdf);
	unsigned int width  = pdf.columns();
	unsigned int height = pdf.rows();

	// Find rotation and output coordinates to x and y
	unsigned int x, y;
	double rotation = findRotation(view, x, y, width, height, pdf);

	if (rotation != 0)
	{
		pdf.draw(DrawableTranslation(-x, -y));
		pdf.rotate(rotation*180.0/pi);
		pdf.trim();
	}

	pdf.write("cow.png");
	
	return 0;
}
