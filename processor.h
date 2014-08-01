/*
 * Thread to process all the forms
 */

#ifndef H_PROCESSOR
#define H_PROCESSOR

#include <list>
#include <mutex>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <podofo/podofo.h>

#include "box.h"
#include "log.h"
#include "read.h"
#include "data.h"
#include "forms.h"
#include "boxes.h"
#include "rotate.h"
#include "pixels.h"
#include "extract.h"
#include "threadqueuevoid.h"

// Called in a new thread for each new form
void extractImages(Form* form);

// Called in a new thread for each image
void parseImage(FormImage* formImage);

// Main class to manage processing the forms
class Processor
{
    // Compare with this to see if we successfully found a form
    Form defaultForm;

    // We need consistent memory locations since we're adding the address to a
    // queue to process as we load each image and form.
    std::list<Form> forms;
    std::mutex forms_mutex;

    // The threads
    ThreadQueueVoid<Form*> extractT;
    ThreadQueueVoid<FormImage*> parseT;

public:
    Processor(int threads)
        : extractT(extractImages, threads),
          parseT(parseImage, threads),
          defaultForm(*this)
    { }

    // Add a new form to be processed
    void add(long long id, long long key, const std::string& filename);

    // Grade the form outputing the result as a string for displaying
    std::string print(long long id);

    // Get reference to either the form with this ID or the defaultForm
    // if this ID doesn't exist
    Form& findForm(long long id);

    // Block till all forms processed
    void wait();

    // Exit all threads
    void exit();

    // Needs to accses mutexes and image list
    friend void extractImages(Form* form);
};

#endif
