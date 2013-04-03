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
Currently this program will accept most multi-page PDFs, find the black boxes,
rotate the image, find the student ID number, find the filled in answers, and
grade based on the form with the teacher's ID. What's left is all the fancy Web
UI, database, and email stuff.

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
Download an [example form](http://floft.net/uploads/freetron_example.pdf)
created from the Apperson form mentioned above. Before I've done any sort of
website frontend, you can grade forms using the terminal. If the teacher's id
is 123456789 and you have a PDF file of the forms, you can run `./freetron -i
123456789 freetron_example.pdf 2>/dev/null` giving the following as output.

    ID          Answers (key first)
    123456789   A B C D E A B C D E A B C D E 
    1233224     A A D D D A B C D E A B B C E 
    987444      A B C D E E D C B A B C D E D 

    Scores
      987444      40.00%
      1233224     66.67%
