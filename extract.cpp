#include "extract.h"

vector<Image> extract(const char* filename)
{
	vector<Image> images;
	PdfObject* pObj = NULL;
	PdfMemDocument document(filename);
	TCIVecObjects it = document.GetObjects().begin();

	while (it != document.GetObjects().end())
	{
		if ((*it)->IsDictionary())
		{
			PdfObject* pObjType = (*it)->GetDictionary().GetKey(PdfName::KeyType);
			PdfObject* pObjSubType = (*it)->GetDictionary().GetKey(PdfName::KeySubtype);

			if ((pObjType    && pObjType->IsName()    && pObjType->GetName().GetName() == "XObject") ||
			    (pObjSubType && pObjSubType->IsName() && pObjSubType->GetName().GetName() == "Image" ))
			{
				pObj = (*it)->GetDictionary().GetKey(PdfName::KeyFilter);

				if (pObj && pObj->IsArray() && pObj->GetArray().GetSize() == 1 &&
				    pObj->GetArray()[0].IsName() && pObj->GetArray()[0].GetName().GetName() == "DCTDecode")
					pObj = &pObj->GetArray()[0];
				
				// See if it's JPEG
				if(pObj && pObj->IsName() && pObj->GetName().GetName() == "DCTDecode")
					images.push_back(readPDFImage(*it, true));
				else
					images.push_back(readPDFImage(*it, false));

				document.FreeObjectMemory(*it);
			}

		}

		++it;
	}

	return images;
}

Image readPDFImage(PdfObject* object, bool isJpeg)
{
	Blob b;
	Image i;
	i.density(Geometry(300,300));
	i.magick("RGB");

	if (isJpeg)
	{
		PdfMemStream* stream = dynamic_cast<PdfMemStream*>(object->GetStream());
		cout << stream->GetLength() << endl;
		b = Blob(stream->Get(), stream->GetLength());
		i.magick("jpeg");
	}
	else
	{
		ostringstream os;
		os << "P6\n# Image extracted by PoDoFo\n"
		   << object->GetDictionary().GetKey( PdfName("Width" ) )->GetNumber() << " "
		   << object->GetDictionary().GetKey( PdfName("Height" ) )->GetNumber() << "\n"
		   << "255\n";
		string s = os.str();

		const char* header = s.c_str();
		char* buffer;
		pdf_long len;

		object->GetStream()->GetFilteredCopy(&buffer, &len);

		char* stream = new char[len+s.size()];
		memcpy(stream, header, s.size());
		memcpy(stream+s.size(), buffer, len);

		b = Blob(stream, len+s.size());
		free(buffer);
		delete[] stream;

		i.magick("ppm");
	}

	i.read(b);
	i.magick("png");

	return i;
}
