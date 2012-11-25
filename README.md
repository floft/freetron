freetron
========

A professor talked to me about creating a software implementation of a
scantron machine, so I here have somewhat working code. Once again it's
licensed under the [ISC license](http://floft.net/uploads/isc-license.txt).
Feel free to try it and improve it. The goal is to scan one PDF of the key
followed by all of the students' sheets, grade, and email (via a database
lookup) the students their grade and a copy of their sheet.

The current dependencies are PoDoFo and graphicsmagick. The readImages function
won't allow changing the density of the images read from the PDF, so I have to
use a different library to extract them. I hope to switch to another library
than graphicsmagick also since it's a bit big when I only need pixel access and
a rotation function.

**Dependencies**  
PoDoFo
graphicsmagick
