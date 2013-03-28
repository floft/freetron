freetron
========

A professor talked to me about creating a software implementation of a scantron
machine, so I here have somewhat working code. Once again it's licensed under
the [ISC license](http://floft.net/uploads/isc-license.txt).  Feel free to try
it and improve it. The goal is to scan one PDF of the key followed by all of
the students' sheets, grade, email (via a database lookup) the students their
grade and a copy of their sheet, and then display statistics about the exam.

For an example of the type of form this works with, you can look at this one
from [Apperson](https://ssl1.appersonsecure.com/pdfs/common/29240.PDF).

**Status**  
Currently this program will accept most multi-page PDFs (some such as the
example PDF above will crash the program), find the black boxes, rotate the
image, find the student ID number, find the filled in answers, and grade based
on the form with the teacher's ID. What's left is all the fancy Web UI,
database, and email stuff.  Although, the algorithm for figuring out what
letter was filled in could be improved.

**Dependencies**  
*a C++11 compiler*  
PoDoFo (LGPL)  
OpenIL/DevIL (LGPL)  
libtiff (custom: http://www.libtiff.org/misc.html)
