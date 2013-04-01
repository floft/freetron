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

Project Status
--------------
Currently this program will accept most multi-page PDFs (some such as the
example PDF above will crash the program), find the black boxes, rotate the
image, find the student ID number, find the filled in answers, and grade based
on the form with the teacher's ID. What's left is all the fancy Web UI,
database, and email stuff.

Using
-----
Since it's not done yet, you can look at the example to see how to run it from
the command line. Eventually, you'll be able to put some PHP files on a webserver
with a freetron daemon running in the background that will monitor a directory
where the webserver will place uploaded forms. Then, after the forms in a queue
have been processed, the grades will be shown on the website. Well, that might be
what happens. We'll see what it looks like when it's done.

**Dependencies**  
*a C++11 compiler*  
PoDoFo (LGPL)  
OpenIL/DevIL (LGPL)  
libtiff (custom: http://www.libtiff.org/misc.html)

Example
-------
Currently, before I've done any sort of website frontend, you can grade forms
using the terminal. If the teacher's id is 01234567 and you have a PDF file of
the forms, you can run `./freetron -i 01234567 forms.pdf 2>/dev/null` giving
the following as output.

    ID          Answers (key first)
    01234567    C C D B C A E D B E A A E D E D B D B B C B C E
    6543210     C B B C D B A D D D B D C C E A C B E C B B _ _
    6543211     C C D B C A E D B E A A E D E D B D B B C B C E

    Scores
      6543210      16.67%
      6543211     100.00%
