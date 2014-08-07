/*
 * Extract the images from the PDF using PoDoFo and libtiff, somewhat similar
 * to podofoimgextract and fax2tiff, a command-line utility
 *
 * Useful stuff for tiffio:
 *   http://www.asmail.be/msg0055289992.html
 *   http://old.nabble.com/CCITTFaxDecode-with-EncodedByteAlign.-How-to-create-a-TIFF-from-that.-td34168750.html
 *   http://stackoverflow.com/questions/4624144/c-libtiff-read-and-save-file-from-and-to-memory
 *   http://stackoverflow.com/questions/4595646/c-decode-ccitt-encoded-images-in-pdfs
 *
 * Got the image to look right from extracted data in the PDF with the options -M4 on fax2tiff,
 * so I looked at the code to see what options were used.
 */

#ifndef H_EXTRACT
#define H_EXTRACT

#include <list>
#include <string>
#include <iostream>
#include <podofo/podofo.h>

#include "forms.h"
#include "pixels.h"

enum class PixelType
{
    Unknown,
    PNM,    // Default
    JPG,    // DCTDecode
    TIF,    // CCITTFaxDecode
};

enum class ColorSpace
{
    Unknown,
    Gray,   // PNM5
    RGB     // PNM6
};

std::list<FormImage> extract(const std::string& filename, Form& form);
Pixels readPDFImage(PoDoFo::PdfObject* object, const PixelType type,
    const ColorSpace colorspace, const std::string& filename);

// Debugging
std::ostream& operator<<(std::ostream& os, const PixelType& t);
std::ostream& operator<<(std::ostream& os, const ColorSpace& c);

#endif
