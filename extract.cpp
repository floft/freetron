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
    PoDoFo::PdfObject* obj = nullptr;
    PoDoFo::PdfObject* color = nullptr;
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

                if (color && color->IsName())
                {
                    std::string col = color->GetName().GetName();

                    if (col == "DeviceRGB")
                        colorspace = ColorSpace::RGB;
                    else if (col == "DeviceGray")
                        colorspace = ColorSpace::Gray;
                }

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

                    if(name == "DCTDecode")
                        pixels = readPDFImage(*it, PixelType::JPG, colorspace, filename);
                    else if (name == "CCITTFaxDecode")
                        pixels = readPDFImage(*it, PixelType::TIF, colorspace, filename);
                    // PNM is the default
                    //else if (name == "FlateDecode")
                    //  pixels = readPDFImage(*it, PixelType::PNM, colorspace, filename);
                    else
                        pixels = readPDFImage(*it, PixelType::PNM, colorspace, filename);
                }
                else
                {
                    pixels = readPDFImage(*it, PixelType::PNM, colorspace, filename);
                }

                document.FreeObjectMemory(*it);
                images.push_back(FormImage(form, std::move(pixels)));
            }
        }

        ++it;
    }

    return images;
}

Pixels readPDFImage(PoDoFo::PdfObject* object, const PixelType type,
    const ColorSpace colorspace, const std::string& filename)
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

        // P5 is graymap (8 bit), P6 is pixmap (24 bit). See:
        // http://en.wikipedia.org/wiki/Portable_anymap#File_format_description
        if (colorspace == ColorSpace::Gray)
            os << "P5\n";
        else
            os << "P6\n";

        os << width  << " "
           << height << "\n"
           << "255\n";
        std::string s = os.str();

        const char* header = s.c_str();
        char* buffer;
        PoDoFo::pdf_long len;

        object->GetStream()->GetFilteredCopy(&buffer, &len);

        // If the buffer isn't the correct size for the image data, don't try
        // reading the image from this invalid data
        if ((colorspace == ColorSpace::Gray && len != width*height) || len != width*height*3)
        {
            std::free(buffer);
            return pixels;
        }

        char* stream = new char[len+s.size()];
        std::memcpy(stream, header, s.size());
        std::memcpy(stream+s.size(), buffer, len);

        pixels = Pixels(IL_PNM, stream, len+s.size(), filename);

        std::free(buffer);
        delete[] stream;
    }

    return pixels;
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
