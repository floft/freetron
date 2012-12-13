#include "extract.h"

vector<Pixels> extract(const char* filename)
{
	vector<Pixels> images;
	PdfObject* obj = nullptr;
	PdfMemDocument document(filename);
	TCIVecObjects it = document.GetObjects().begin();

	while (it != document.GetObjects().end())
	{
		if ((*it)->IsDictionary())
		{
			PdfObject* objType = (*it)->GetDictionary().GetKey(PdfName::KeyType);
			PdfObject* objSubType = (*it)->GetDictionary().GetKey(PdfName::KeySubtype);

			if ((objType    && objType->IsName()    && objType->GetName().GetName() == "XObject") ||
			    (objSubType && objSubType->IsName() && objSubType->GetName().GetName() == "Image" ))
			{
				obj = (*it)->GetDictionary().GetKey(PdfName::KeyFilter);

				if (obj && obj->IsArray() && obj->GetArray().GetSize() == 1 &&
				    obj->GetArray()[0].IsName() && obj->GetArray()[0].GetName().GetName() == "DCTDecode")
					obj = &obj->GetArray()[0];
				
				if(obj && obj->IsName() && obj->GetName().GetName() == "DCTDecode")
					images.push_back(readPDFImage(*it, IL_JPG));
				else if (obj && obj->IsName() && obj->GetName().GetName() == "CCITTFaxDecode")
					images.push_back(readPDFImage(*it, IL_TIF));
				else
					images.push_back(readPDFImage(*it, IL_PNM));

				document.FreeObjectMemory(*it);
			}

		}

		++it;
	}

	return images;
}

Pixels readPDFImage(PdfObject* object, const unsigned int type)
{
	Pixels pixels;
	const unsigned int width  = object->GetDictionary().GetKey(PdfName("Width"))->GetNumber();
	const unsigned int height = object->GetDictionary().GetKey(PdfName("Height"))->GetNumber();

	if (type == IL_JPG)
	{
		PdfMemStream* stream = dynamic_cast<PdfMemStream*>(object->GetStream());
		pixels = Pixels(type, stream->Get(), stream->GetLength());
	}
	else if (type == IL_TIF)
	{
		// Monochrome, otherwise wouldn't have used CCITT
		const unsigned int bits = 1;
		const unsigned int samples = 1;
		
		ostringstream os;
		TIFF* tif = TIFFStreamOpen("Input", &os);
		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,		width);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH,		height);
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,	bits);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL,	samples);
		TIFFSetField(tif, TIFFTAG_FILLORDER,		FILLORDER_MSB2LSB);
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG,		PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,		PHOTOMETRIC_MINISWHITE);
		TIFFSetField(tif, TIFFTAG_XRESOLUTION,		204.0); // From fax2tiff
		TIFFSetField(tif, TIFFTAG_YRESOLUTION,		196.0); // ditto
		TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT,	RESUNIT_INCH);
		TIFFSetField(tif, TIFFTAG_COMPRESSION, 		COMPRESSION_CCITTFAX4);
		TIFFSetField(tif, TIFFTAG_ORIENTATION,		ORIENTATION_TOPLEFT);
		TIFFSetField(tif, TIFFTAG_FAXMODE,		FAXMODE_CLASSF);
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,		(uint32)-1L);
		
		// stream->Get returns read-only, so copy it
		PdfMemStream* stream = dynamic_cast<PdfMemStream*>(object->GetStream());
		const char* readonly = stream->Get();
		pdf_long len = stream->GetLength();
		char* buffer = new char[len];
		memcpy(buffer, readonly, len);

		TIFFWriteRawStrip(tif, 0, buffer, len);
		TIFFWriteDirectory(tif);
		TIFFClose(tif);

		delete[] buffer;

		pixels = Pixels(type, os.str().c_str(), os.tellp());
	}
	else
	{
		ostringstream os;
		os << "P6\n"
		   << width  << " "
		   << height << "\n"
		   << "255\n";
		string s = os.str();

		const char* header = s.c_str();
		char* buffer;
		pdf_long len;

		object->GetStream()->GetFilteredCopy(&buffer, &len);

		char* stream = new char[len+s.size()];
		memcpy(stream, header, s.size());
		memcpy(stream+s.size(), buffer, len);

		pixels = Pixels(type, stream, len+s.size());
		free(buffer);
		delete[] stream;
	}

	return pixels;
}
