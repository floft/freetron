/*
 * Extract the images from the PDF using PoDoFo, similar to
 * podofoimgextract, a command-line utility
 * 
 * Useful stuff for tiffio:
 *   http://www.asmail.be/msg0055289992.html
 *   http://old.nabble.com/CCITTFaxDecode-with-EncodedByteAlign.-How-to-create-a-TIFF-from-that.-td34168750.html
 *   http://stackoverflow.com/questions/4624144/c-libtiff-read-and-save-file-from-and-to-memory
 */

#ifndef H_EXTRACT
#define H_EXTRACT

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <tiffio.h>
#include <tiffio.hxx>
#include <podofo/podofo.h>

#include "pixels.h"

std::vector<Pixels> extract(const char* filename);
Pixels readPDFImage(PoDoFo::PdfObject* object, const unsigned int type);

#endif
