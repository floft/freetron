/*
 * Extract the images from the PDF using PoDoFo, similar to
 * podofoimgextract, a command-line utility
 */

#ifndef H_EXTRACT
#define H_EXTRACT

#include <iostream> //TODO: REMOVE THIS
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <podofo/podofo.h>
#include <Magick++.h>

using namespace std;
using namespace PoDoFo;
using namespace Magick;

vector<Image> extract(const char* filename);
Image readPDFImage(PdfObject* object, bool isJpeg);

#endif
