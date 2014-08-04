freetron
========
A professor encouraged me to write a software implementation of a scantron
machine. It's licensed under the [ISC](http://floft.net/uploads/isc-license.txt)
or [MIT](http://floft.net/uploads/isc-license.txt) licenses. The goal was to scan
a PDF of the key and all the students' sheets, grade, email the students their
grade and a copy of their sheet, and then display statistics about the exam.
While the program doesn't quite do all that, it does grade scanned forms
and provide a website interface for convenience. Feel free to try it out and
improve it.

At the moment this only supports one type of form available directly from
[Apperson](https://ssl1.appersonsecure.com/pdfs/common/29240.PDF) or in
*website/files/form.pdf*.

Using
-----
You can either use this as a command-line utility or run it as a daemon
providing a website interface.

###Dependencies###
*a C++11 compiler*  
CppDB (Boost or MIT)  
CppCMS (LGPL)  
PoDoFo (LGPL)  
OpenIL/DevIL (LGPL)  
libtiff (custom: http://www.libtiff.org/misc.html)  

###Compiling###
**make:** ``make``  
**cmake:** ``cd cmake; cmake .; make``

I have tested g++ and clang++ on Linux. For Mac you'll find most of the
dependencies in Macports, Fink, Homebrew, or whatever you use, but you'll
probably have to build CppCMS and CppDB. On Windows you'll have to build
basically all of these.

###Configuring Website###
Rename *website/config.js.example* to *website/config.js*. Run
``cppcms_make_key --hmac sha256 --cbc aes`` and copy the keys into the
*website/config.js* file. Change the port.

###Running###
Website interface: ``./freetron --daemon website/``  
Command line interface: ``./freetron -i KeyID form.pdf``

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
