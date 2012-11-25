/*
 * Extract the images from the PDF using PoDoFo, similar to
 * podofoimgextract, a command-line utility
 */

#ifndef H_EXTRACT
#define H_EXTRACT

#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <podofo/podofo.h>

#include "pixels.h"

using namespace std;
using namespace PoDoFo;

vector<Pixels> extract(const char* filename);
Pixels readPDFImage(PdfObject* object, bool isJpeg);

#endif
