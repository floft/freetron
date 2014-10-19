/*
 * Thread to process all the forms
 */

#ifndef H_PROCESSOR
#define H_PROCESSOR

#include <list>
#include <mutex>
#include <atomic>
#include <string>
#include <condition_variable>

#include "forms.h"
#include "threadqueuevoid.h"
#include "website/database.h"

// Called in a new thread for each new form
void extractImages(Form* form);

// Called in a new thread for each image
void parseImage(FormImage* formImage);

// For sending status updates
struct Status
{
    long long formId;
    int percentage;

    Status(long long formId, int percentage)
        : formId(formId), percentage(percentage)
    {
    }
};

// Main class to manage processing the forms
class Processor
{
    // Compare with this to see if we successfully found a form
    Form defaultForm;

    // Set if we want to wait or exit
    std::atomic_bool exiting;
    std::atomic_bool waiting;

    // We need consistent memory locations since we're adding the address to a
    // queue to process as we load each image and form.
    std::list<Form> forms;
    std::mutex forms_mutex;

    // The threads
    ThreadQueueVoid<Form*> extractT;
    ThreadQueueVoid<FormImage*> parseT;

    // We need to add the new forms to this database
    Database& db;

    // Do we want to delete the forms after we're done?
    // We do on the site, not for CLI usage
    bool website;

    // Updated whenever an image is done being processed
    std::mutex status_mutex;
    int statusWaiting;
    bool statusNewItem;
    std::vector<Status> status;
    std::condition_variable statusCond;

public:
    Processor(int threads, bool website, Database& db);
    ~Processor();

    // Add a new form to be processed
    void add(long long id, long long key, const std::string& filename);

    // Return if it's done yet (i.e., the form no longer exists)
    bool done(long long id);

    // Get the results and delete the form (only for use with daemon)
    std::string get(long long id);

    // Grade the form outputing the result as a string for displaying
    std::string print(Form& form);
    std::string print(long long id);

    // Display results in a CSV file
    std::string csv(Form& form);
    std::string csv(long long id);

    // Block till all forms processed
    void wait();

    // Exit all threads
    void exit();

    // Block until more forms have been processed returning all of the status
    // updates, which may include previous ones if another form finished being
    // processed before all the previous status updates were grabbed
    std::vector<Status> statusWait();

    // Fake that there is a new status item, e.g. use it to wake up all blocked
    // statusWait() calls so that you can restart the website
    void statusWakeAll();

private:
    // Get reference to either the form with this ID or the defaultForm
    // if this ID doesn't exist
    Form& findForm(long long id);

    // Finish processing the form, add it to the database, delete the PDF
    void finish(long long id);

    // Needs to accses mutexes and image list
    friend void extractImages(Form*);

    // Needs to access finish()
    friend void parseImage(FormImage*);

    // Another form is done
    void statusAdd(const Status& newStatus);
};

#endif
