/*
 * Data structures for forms
 */

#ifndef H_FORMS
#define H_FORMS

#include <list>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>

#include "log.h"
#include "data.h"
#include "pixels.h"

struct Form;
class Processor;

// For each image in each PDF
struct FormImage
{
    // Image to process
    Pixels image;

    // Reference to parent, used to log messages
    Form& form;

    // Processed data
    long long id;
    long long thread_id;
    std::vector<Answer> answers;

    FormImage(Form& form, Pixels&& image)
        : image(image), form(form), id(-1), thread_id(-1)
    { }
};

// Created for each new PDF to process
struct Form
{
    long long id;
    long long key;
    long long pages;
    std::string filename;

    // Forms processed
    long long done;
    std::mutex done_mutex;

    // For thread-safe log messages
    std::string output;
    std::mutex output_mutex;

    // The images in the form
    std::list<FormImage> formImages;
    std::mutex images_mutex;

    // Reference to parent
    Processor& processor;

    Form(Processor& processor)
        : id(-1), key(0), pages(-1), done(0), processor(processor)
    { }

    Form(Form&&);

    Form(long long id, long long key, const std::string& filename,
            Processor& processor)
        : id(id), key(key), pages(-1), filename(filename), done(0), processor(processor)
    {
    }

    // Increment or get how many forms we've done
    void incDone();
    long long getDone();

    void log(const std::string& msg, const LogType& t = LogType::Error);
};

// Used for searching for the form with a certain ID
class FormPredicate
{
    long long id;

public:
    FormPredicate(long long id)
        : id(id)
    {
    }

    bool operator()(const Form& f)
    {
        return f.id == id;
    }
};

// Equality based on if IDs are equal
bool operator==(const Form& a, const Form& b);
bool operator!=(const Form& a, const Form& b);

#endif
