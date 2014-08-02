/*
 * Thread to process all the forms
 */

#ifndef H_PROCESSOR
#define H_PROCESSOR

#include <list>
#include <mutex>
#include <string>
#include <cstdio>
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
#include "math.h"
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

    // Set if we want to die
    std::atomic_bool exiting;

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
          defaultForm(*this),
          exiting(false)
    { }

    // Add a new form to be processed
    void add(long long id, long long key, const std::string& filename);

    // If done or doesn't exist, return 100
    // If not, wait for next page to complete, return new percentage
    int statusWait(long long id);

    // Return if it's done yet
    bool done(long long id);

    // Grade the form outputing the result as a string for displaying
    std::string print(Form& form);
    std::string print(long long id);

    // Get the results and delete the form (only for use with daemon)
    std::string get(long long id);

    // Block till all forms processed
    void wait();

    // Exit all threads
    void exit();

    // Needs to accses mutexes and image list
    friend void extractImages(Form* form);

private:
    // Get reference to either the form with this ID or the defaultForm
    // if this ID doesn't exist
    Form& findForm(long long id);
};

#endif
