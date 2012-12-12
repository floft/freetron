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

	if (type == IL_JPG)
	{
		cout << "jpeg" << endl;
		PdfMemStream* stream = dynamic_cast<PdfMemStream*>(object->GetStream());
		pixels = Pixels(type, stream->Get(), stream->GetLength());
	}
	else if (type == IL_TIF)
	{
		cout << "tif" << endl;
		//PdfMemStream* stream = dynamic_cast<PdfMemStream*>(object->GetStream());
		//PdfFilteredDecodeStream decode(stream, ePdfFilter_CCITTFaxDecode, true);
		//decode.Write();

		char* buffer;
		pdf_long len;

		object->GetStream()->GetFilteredCopy(&buffer, &len);
		cout << len << endl;

		//char* stream = new char[len+s.size()];
		//memcpy(stream, header, s.size());
		//memcpy(stream+s.size(), buffer, len);

		pixels = Pixels(type, buffer, len);
		free(buffer);
		//delete[] stream;

		//pixels = Pixels(type, stream->Get(), stream->GetLength());
		/*
		//ostringstream os;
		ofstream os("cow.tif", ofstream::out);

		const unsigned int bits = 1;
		const unsigned int samples = 1;
		const unsigned int width  = object->GetDictionary().GetKey(PdfName("Width"))->GetNumber();
		const unsigned int height = object->GetDictionary().GetKey(PdfName("Height"))->GetNumber();
		
		TIFF* tif = TIFFStreamOpen("MemTIFF", &os);
		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, width*samples));
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bits);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samples);
		//TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, object->GetDictionary().GetKey(PdfName("Width"))->GetNumber());
		TIFFSetField(tif, TIFFTAG_T4OPTIONS, GROUP3OPT_FILLBITS);
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_CCITTRLE);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
		TIFFSetField(tif, TIFFTAG_FAXMODE, FAXMODE_BYTEALIGN|FAXMODE_CLASSF);
		
		PdfMemStream* stream2 = dynamic_cast<PdfMemStream*>(object->GetStream());

		if (stream2 == NULL)
			cout << "stream is null" << endl;
		else
			cout << "stream is not null" << endl;

		//TIFFWriteRawStrip(tif, 0, stream, stream->GetLength());
		unsigned char* stream = new unsigned char[78808];
		FILE* raw = fopen("images/out.raw", "r");
		fread(stream, 1, 78808, raw);
		fclose(raw);
		TIFFWriteRawStrip(tif, 0, stream, 78808);
		TIFFClose(tif);*/

		//pixels = Pixels(type, os.str().c_str(), os.tellp());
	}
	else
	{
		cout << "ppm" << endl;
		ostringstream os;
		os << "P6\n"
		   << object->GetDictionary().GetKey(PdfName("Width"))->GetNumber() << " "
		   << object->GetDictionary().GetKey(PdfName("Height"))->GetNumber() << "\n"
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
