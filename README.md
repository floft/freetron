freetron
========

A professor talked to me about creating a software implementation of a
scantron machine, so I here have somewhat working code. Once again it's
licensed under the [ISC license](http://floft.net/uploads/isc-license.txt).
Feel free to try it and improve it. The goal is to scan one PDF of the key
followed by all of the students' sheets, grade, email (via a database
lookup) the students their grade and a copy of their sheet, and then display
statistics about the exam.

**Dependencies**  
PoDoFo (LGPL)  
OpenIL/DevIL (LGPL) 
libtiff (custom: http://www.libtiff.org/misc.html)
