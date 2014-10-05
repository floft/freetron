#include <memory>
#include <sstream>
#include <fstream>
#include <cstring>
#include <IL/il.h>
#include <tiffio.h>
#include <tiffio.hxx>

#include "extract.h"

std::list<FormImage> extract(const std::string& filename, Form& form)
{
    std::list<FormImage> images;
    ColorSpace colorspace;
    PoDoFo::pdf_int64 componentbits;
    PoDoFo::PdfObject* obj = nullptr;
    PoDoFo::PdfObject* color = nullptr;
    PoDoFo::PdfObject* component = nullptr;
    PoDoFo::PdfMemDocument document(filename.c_str());
    PoDoFo::TCIVecObjects it = document.GetObjects().begin();

    while (it != document.GetObjects().end())
    {
        if ((*it)->IsDictionary())
        {
            PoDoFo::PdfObject* objType = (*it)->GetDictionary().GetKey(PoDoFo::PdfName::KeyType);
            PoDoFo::PdfObject* objSubType = (*it)->GetDictionary().GetKey(PoDoFo::PdfName::KeySubtype);

            if ((objType    && objType->IsName()    && objType->GetName().GetName() == "XObject") ||
                (objSubType && objSubType->IsName() && objSubType->GetName().GetName() == "Image" ))
            {
                // Colorspace
                color = (*it)->GetDictionary().GetKey(PoDoFo::PdfName("ColorSpace"));
                colorspace = ColorSpace::Unknown;

                if (color && color->IsReference())
                    color = document.GetObjects().GetObject(color->GetReference());

                // Follow ICCBased reference to the Alternate colorspace
                if (color && color->IsArray() && color->GetArray().GetSize() == 2 &&
                        // First item is ICCBased
                        color->GetArray()[0].IsName() &&
                        color->GetArray()[0].GetName().GetName() == "ICCBased" &&
                        // Second item is reference to color space
                        color->GetArray()[1].IsReference())
                {
                    color = document.GetObjects().GetObject(color->GetArray()[1].GetReference());

                    if (color)
                        color = color->GetDictionary().GetKey(PoDoFo::PdfName("Alternate"));
                }

                // Check if either RGB or Grayscale (either the specified
                // colorspace or the alternate if using an ICCBased colorspace)
                if (color && color->IsName())
                {
                    std::string col = color->GetName().GetName();

                    if (col == "DeviceRGB")
                        colorspace = ColorSpace::RGB;
                    else if (col == "DeviceGray")
                        colorspace = ColorSpace::Gray;
                }

                // Bits per component
                component = (*it)->GetDictionary().GetKey(PoDoFo::PdfName("BitsPerComponent"));
                componentbits = 8;

                if (component && component->IsNumber())
                    componentbits = component->GetNumber();

                // Stream
                obj = (*it)->GetDictionary().GetKey(PoDoFo::PdfName::KeyFilter);

                // JPEG and Flate are in another array
                if (obj && obj->IsArray() && obj->GetArray().GetSize() == 1 &&
                    ((obj->GetArray()[0].IsName() && obj->GetArray()[0].GetName().GetName() == "DCTDecode") ||
                     (obj->GetArray()[0].IsName() && obj->GetArray()[0].GetName().GetName() == "FlateDecode")))
                    obj = &obj->GetArray()[0];

                Pixels pixels;

                if (obj && obj->IsName())
                {
                    std::string name = obj->GetName().GetName();

                    if (name == "DCTDecode")
                        pixels = readPDFImage(*it, PixelType::JPG, colorspace, componentbits, filename, form);
                    else if (name == "CCITTFaxDecode")
                        pixels = readPDFImage(*it, PixelType::TIF, colorspace, componentbits, filename, form);
                    // PNM is the default
                    //else if (name == "FlateDecode")
                    //  pixels = readPDFImage(*it, PixelType::PNM, colorspace, componentbits, filename, form);
                    else
                        pixels = readPDFImage(*it, PixelType::PNM, colorspace, componentbits, filename, form);
                }
                else
                {
                    pixels = readPDFImage(*it, PixelType::PNM, colorspace, componentbits, filename, form);
                }

                document.FreeObjectMemory(*it);

                if (pixels.isLoaded())
                    images.push_back(FormImage(form, std::move(pixels)));
            }
        }

        ++it;
    }

    return images;
}

Pixels readPDFImage(PoDoFo::PdfObject* object, const PixelType type,
    const ColorSpace colorspace, const PoDoFo::pdf_int64 componentbits,
    const std::string& filename, Form& form)
{
    Pixels pixels;

    if (!object->GetDictionary().HasKey(PoDoFo::PdfName("Width")) ||
        !object->GetDictionary().HasKey(PoDoFo::PdfName("Height")))
        return pixels;

    const unsigned int width  = object->GetDictionary().GetKey(PoDoFo::PdfName("Width"))->GetNumber();
    const unsigned int height = object->GetDictionary().GetKey(PoDoFo::PdfName("Height"))->GetNumber();

    if (type == PixelType::JPG)
    {
        PoDoFo::PdfMemStream* stream = dynamic_cast<PoDoFo::PdfMemStream*>(object->GetStream());
        pixels = Pixels(IL_JPG, stream->Get(), stream->GetLength(), filename);
    }
    else if (type == PixelType::TIF)
    {
        // Monochrome, otherwise wouldn't have used CCITT
        const unsigned int bits = 1;
        const unsigned int samples = 1;

        if (bits != componentbits)
            form.log("BitsPerComponent is not 1 for CCITTFaxDecode image in PDF", LogType::Warning);

        std::ostringstream os;
        TIFF* tif = TIFFStreamOpen("Input", &os);
        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,       width);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH,      height);
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,    bits);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL,  samples);
        TIFFSetField(tif, TIFFTAG_FILLORDER,        FILLORDER_MSB2LSB);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG,     PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,      PHOTOMETRIC_MINISWHITE);
        TIFFSetField(tif, TIFFTAG_XRESOLUTION,      204.0); // From fax2tiff
        TIFFSetField(tif, TIFFTAG_YRESOLUTION,      196.0); // ditto
        TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT,   RESUNIT_INCH);
        TIFFSetField(tif, TIFFTAG_COMPRESSION,      COMPRESSION_CCITTFAX4);
        TIFFSetField(tif, TIFFTAG_ORIENTATION,      ORIENTATION_TOPLEFT);
        TIFFSetField(tif, TIFFTAG_FAXMODE,      FAXMODE_CLASSF);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,     (uint32)-1L);

        // stream->Get returns read-only, so copy it
        PoDoFo::PdfMemStream* stream = dynamic_cast<PoDoFo::PdfMemStream*>(object->GetStream());
        const char* readonly = stream->Get();
        PoDoFo::pdf_long len = stream->GetLength();
        char* buffer = new char[len];
        std::memcpy(buffer, readonly, len);

        TIFFWriteRawStrip(tif, 0, buffer, len);
        TIFFWriteDirectory(tif);
        TIFFClose(tif);

        delete[] buffer;

        pixels = Pixels(IL_TIF, os.str().c_str(), os.tellp(), filename);
    }
    else
    {
        std::ostringstream os;

        // P4 is bitmap (1 bit), P5 is graymap (8 bit), P6 is pixmap (24 bit). See:
        // http://netpbm.sourceforge.net/doc/pbm.html
        // http://en.wikipedia.org/wiki/Portable_anymap#File_format_description
        if (colorspace == ColorSpace::Gray)
            os << ((componentbits == 1)?"P4\n":"P5\n");
        else
            os << "P6\n";

        os << width  << " "
           << height << "\n"
           << "255\n";
        std::string s = os.str();

        char* buffer;
        PoDoFo::pdf_long len;

        object->GetStream()->GetFilteredCopy(&buffer, &len);

        // Warn if unknown colorspace
        if (colorspace == ColorSpace::Unknown)
            form.log("unknown color space on PDF image", LogType::Warning);

        // If the buffer isn't the correct size for the image data, don't try
        // reading the image from this invalid data
        if (len != correctLength(width, height, colorspace, componentbits))
        {
            std::ostringstream ss;
            ss << "wrong buffer size for PDF image of size "
               << width << "x" << height
               << " (" << colorspace << "): " << len;
            form.log(ss.str(), LogType::Warning);

            std::free(buffer);
            return pixels;
        }

        std::unique_ptr<char> stream(new char[len+s.size()]);
        std::memcpy(stream.get(), s.c_str(), s.size());
        std::memcpy(stream.get()+s.size(), buffer, len);
        std::free(buffer);

        pixels = Pixels(IL_PNM, stream.get(), len+s.size(), filename);
    }

    return pixels;
}

// Determine the correct length of the image data buffer depending on the color
// space, bit depth, etc.
long long correctLength(const int width, const int height,
        const ColorSpace colorspace,
        const PoDoFo::pdf_int64 componentbits)
{
    if (colorspace == ColorSpace::Gray)
    {
        // 1-bit: each pixel is one bit, with padding at ends of rows
        if (componentbits == 1)
        {
            int r = width%8;
            int fileWidth = width;

            if (r != 0)
                fileWidth = fileWidth - r + 8;

            return fileWidth*height/8;
        }
        // 8-bit: each pixel is a byte
        else
        {
            return width*height;
        }
    }
    else
    {
        // 8-bit per channel: pixel is 3 bytes
        return width*height*3;
    }

    return -1;
}

// Debugging
std::ostream& operator<<(std::ostream& os, const PixelType& t)
{
    switch (t)
    {
        case PixelType::Unknown: return os << "Unknown";
        case PixelType::PNM: return os << "PNM";
        case PixelType::JPG: return os << "JPG";
        case PixelType::TIF: return os << "TIF";
        default: return os << "Unknown?";
    }
}

std::ostream& operator<<(std::ostream& os, const ColorSpace& c)
{
    switch (c)
    {
        case ColorSpace::Unknown: return os << "Unknown";
        case ColorSpace::Gray: return os << "Gray";
        case ColorSpace::RGB: return os << "RGB";
        default: return os << "Unknown?";
    }
}
