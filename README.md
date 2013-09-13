freetron
========
A professor encouraged me to write a software implementation of a scantron
machine. It's licensed under the [ISC
license](http://floft.net/uploads/isc-license.txt). The goal was to scan a PDF
of the key and all the students' sheets, grade, email the students their grade
and a copy of their sheet, and then display statistics about the exam. While
the program doesn't quite do all that, it does grade scanned forms, the
fundamental part. Feel free to try it out and improve it.

For an example of the type of form this works with, you can look at this one
from [Apperson](https://ssl1.appersonsecure.com/pdfs/common/29240.PDF).

Project Status
--------------
Currently this multi-threaded program will accept most multi-page PDFs, find
the black boxes, rotate the image, find the student ID number, find the filled
in answers, and grade based on the form with the teacher's ID. What's left is
all the fancy Web UI, database, email stuff, and extensive testing (although
I've tested it with over 50 forms).

Using
-----
As of now there's no GUI, so you can look at the example to see how to run it
from the command line. You need to install the following dependencies and
compile it with either make or cmake. You can also use the provided Qt Creator
project file if you prefer an IDE.

###Dependencies###
*a C++11 compiler*  
PoDoFo (LGPL)  
OpenIL/DevIL (LGPL)  
libtiff (custom: http://www.libtiff.org/misc.html)

I have tested g++ and clang++ on Linux. I haven't tested it on Windows yet
since I'd have to compile PoDoFo.  All the dependencies above are in Macports,
so compiling it on Mac should be fairly straightforward.

###Compiling###
**make:** ``make``  
**cmake:** ``cd cmake; cmake .; make``

Note that cmake will probably look prettier, giving more helpful error
messages.

Example
-------
Download a [computer-generated
form](http://floft.net/uploads/freetron_example.pdf) or a [scanned
form](http://floft.net/uploads/freetron_example2.pdf) created from the Apperson
form mentioned above. Below is the output of these two example forms. The first
takes about 1 second and the second about 3 seconds on my computer.

    $ ./freetron -i 123456789 freetron_example.pdf 2>/dev/null
    ID          Answers (key first)
    123456789   A B C D E A B C D E A B C D E 
    1233224     A A D D D A B C D E A B B C E 
    987444      A B C D E E D C B A B C D E D 

    Scores
      987444      40.00%
      1233224     66.67%

    $ ./freetron -i 1793240 freetron_example2.pdf 2>/dev/null
    ID          Answers (key first)
    1793240     E D B B C A B D C B D A D E E B D C E C A A E C C D C C C E C E E D D A C B E A D E A A A C D A E A
    888324401   D D B B C C B D A B D A E E E B D C B C A B E C D D C C E E C D D D A A C A A A D A A B A C D A E A
    3741083751  _ C C C B A B D C B C A D D E A D D E A A A E C D D E A C B C E E E D C C E E C C E B A _ E B A E B

    Scores
      888324401   70.00%
      3741083751  52.00%

If you specify the *-d* option, it'll generate debug images, which allows you
to find any problematic bubbles or boxes. Below is the output from the third
page of the example scanned form. Note that since OpenIL doesn't allow
multi-threading, this greatly extends the run time.

[![Freetron Debug Image](http://floft.net/uploads/freetron_debug.jpg)](http://floft.net/uploads/freetron\_debug.png)
