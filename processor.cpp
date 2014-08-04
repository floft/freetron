#include "processor.h"

Processor::Processor(int threads, bool website, Database& db)
    : defaultForm(*this),
      exiting(false),
      waiting(false),
      extractT(extractImages, threads),
      parseT(parseImage, threads),
      db(db),
      website(website)
{
}

Processor::~Processor()
{
    exit();
}

void extractImages(Form* form)
{
    std::list<FormImage> newImages;

    try
    {
        // Get the images from the PDF
        newImages = extract(form->filename, *form);

        // Set number of pages to however many images we got out of the PDF
        form->pages = newImages.size();
    }
    catch (const std::runtime_error& error)
    {
        form->log(form->filename + ", " + error.what());
        form->processor.finish(form->id);
        return;
    }
    catch (const PoDoFo::PdfError& error)
    {
        //error.PrintErrorMsg();
        //return error.GetError();

        // Don't write this to screen
        form->log(form->filename + ", " + error.what(), LogType::Error);
        form->processor.finish(form->id);
        return;
    }
    catch (...)
    {
        form->log("Unhandled exception");
        form->processor.finish(form->id);
        return;
    }

    // If no images, we're done
    if (newImages.empty())
        form->processor.finish(form->id);

    // Add these extracted images to the queue after making sure there aren't
    // any extraction errors
    for (FormImage& i : newImages)
        form->processor.parseT.queue(&i);

    // Move these new images to the actual list. Do this here instead
    // of directly appending them to the list so that we only need
    // to lock this once.
    std::unique_lock<std::mutex> lock(form->images_mutex);
    form->formImages.splice(form->formImages.end(), newImages);
}

void parseImage(FormImage* formImage)
{
    // Use this to get a unique ID each time this function is called,
    // used for writing out the debug images
    static long long static_thread_id = 0;
    const long long thread_id = static_thread_id++;

    // When this thread has an error, write message including thread id, but
    // continue processing the rest of the images.
    try
    {
        // Find all blobs in the image
        Blobs blobs(formImage->image);

        // Box information for this image
        Data data;

        // Find all the boxes
        std::vector<Coord> boxes = findBoxes(formImage->image, blobs, data);

        if (boxes.size() > TOTAL_BOXES)
            throw std::runtime_error("too many boxes detected");

        if (boxes.size() < TOTAL_BOXES)
            throw std::runtime_error("some boxes not detected");

        if (DEBUG)
        {
            std::ostringstream s_orig;
            s_orig << "debug" << thread_id << "_orig.png";
            formImage->image.save(s_orig.str(), true, false, true);
        }

        // Rotate the image
        Coord rotate_point;
        double rotation = findRotation(formImage->image, boxes, rotate_point);

        // Negative since the origin is the top-left point (this is the 4th quadrant)
        if (rotation != 0)
        {
            formImage->image.rotate(-rotation, rotate_point);
            formImage->image.rotateVector(boxes, rotate_point, -rotation);

            // The blobs are constant, so just recalculate them all
            blobs = Blobs(formImage->image);
        }

        // Sort all boxes if with respect to y if we rotated counter-clockwise
        if (rotation > 0)
            std::sort(boxes.begin(), boxes.end());

        // Sort the bottom row of boxes by increasing x coordinates.
        std::sort(boxes.begin()+BOT_START-1, boxes.begin()+BOT_END, CoordXSort());

        // Determine what is black (changes in BW, color, and grayscale)
        double black = findBlack(formImage->image, blobs, boxes, data);

        // Find ID number
        long long id = findID(formImage->image, blobs, boxes, data, black);

        std::vector<Answer> answers;

        // Don't bother finding answers if we couldn't even get the student ID
        if (id != DefaultID)
            answers = findAnswers(formImage->image, blobs, boxes, data, black);

        // Debug information
        if (DEBUG)
        {
            for (const Coord& box : boxes)
                formImage->image.mark(box);

            std::ostringstream s;
            s << "debug" << thread_id << ".png";
            formImage->image.save(s.str());
        }

        formImage->id = id;
        formImage->answers = answers;
        formImage->thread_id = thread_id;
    }
    catch (const std::runtime_error& error)
    {
        if (DEBUG)
        {
            std::ostringstream s;
            s << "debug" << thread_id << ".png";
            formImage->image.save(s.str());
        }

        std::ostringstream msg;
        msg << "thread #" << thread_id << ", " << formImage->image.filename()
            << " - " << error.what();
        formImage->form.log(msg.str());
    }

    // Another page is complete
    formImage->form.incDone();

    // If done, save results
    if (formImage->form.getDone() == formImage->form.pages)
        formImage->form.processor.finish(formImage->form.id);
}

void Processor::wait()
{
    waiting = true;
    extractT.wait();
    parseT.wait();
}

void Processor::exit()
{
    exiting = true;

    // Only exit these threads if we haven't already waited for them to complete
    // (thus they already exited)
    if (!waiting)
    {
        extractT.exit();
        parseT.exit();
    }
}

Form& Processor::findForm(long long id)
{
    std::unique_lock<std::mutex> lock(forms_mutex);
    std::list<Form>::iterator i = std::find_if(forms.begin(), forms.end(),
            FormPredicate(id));

    if (i == forms.end())
        return defaultForm;
    else
        return *i;
}

bool Processor::done(long long id)
{
    Form& form = findForm(id);
    return form == defaultForm || form.finished;
}

int Processor::statusWait(long long id)
{
    Form& form = findForm(id);

    if (form == defaultForm || form.finished || form.pages == 0)
        return 100;

    std::unique_lock<std::mutex> lck(form.done_mutex);
    long long done = form.done;

    // Wait for update
    while (true)
    {
        form.waitCond.wait(lck, [this, &form, done] {
            return form.done == form.pages || form.done != done || exiting;
        });

        if (form.done == form.pages || form.done != done || exiting)
            break;
    }

    // Now we're definitely "done"
    if (exiting)
        return 100;

    // Return new percentage
    int percentage = smartFloor(100.0*form.done/form.pages);

    // Return 99 if it's "done," the last percent is adding it to
    // the database, etc.
    return (percentage==100)?99:percentage;
}

void Processor::finish(long long id)
{
    // Only do this when used with the website. We don't want to delete
    // forms passed in as command line arguments.
    if (!website)
        return;

    std::string out;
    std::string filename;

    {
        std::unique_lock<std::mutex> lock(forms_mutex);
        std::list<Form>::iterator form = std::find_if(forms.begin(), forms.end(),
                FormPredicate(id));

        // Not found
        if (form == forms.end())
            return;

        // Wake up anybody still waiting for this setting finished to true
        // so they will all exit before we delete this form.
        form->finished = true;
        form->waitCond.notify_all();

        // Get output
        out = print(*form);
        filename = form->filename;

        {
            // Hopefully wait till the last website thread accesses the form
            std::unique_lock<std::mutex> lck(form->done_mutex);
        }

        // Delete from list
        forms.erase(form);
    }

    // Save to database
    db.updateForm(id, out);

    // Delete off disk last, if we get killed right before doing this,
    // we'll see this file still exists and reprocess this form on start
    if (std::remove(filename.c_str()) != 0)
        log("Couldn't delete form \"" + filename + "\"");
}

std::string Processor::print(long long id)
{
    Form& form = findForm(id);
    return print(form);
}

std::string Processor::print(Form& form)
{
    std::ostringstream out;

    if (form == defaultForm)
        return "";

    // Find the key based on the key's ID
    out << std::left;

    if (DEBUG)
        out << std::setw(5)  << "#";

    out << std::setw(10) << "ID"
        << "\tAnswers (key first)"
        << std::endl;

    int total = 0;
    bool found = false;
    std::vector<Answer> key;

    for (const FormImage& i : form.formImages)
    {
        if (i.id == form.key)
        {
            if (found)
            {
                log("key already found, skipping this one");
            }
            else
            {
                if (DEBUG)
                    out << std::setw(5) << i.thread_id;

                out << std::setw(10) << i.id << "\t";

                for (const Answer b : i.answers)
                {
                    if (b != Answer::Blank)
                    {
                        out << b << " ";
                        ++total;
                    }
                }

                out << std::endl;

                key = i.answers;
                found = true;
            }
        }
    }

    if (found)
    {
        typedef std::vector<Answer>::size_type size_type;

        // Grade student's exams
        std::map<long long, double> scores;

        for (const FormImage& i : form.formImages)
        {
            if (i.id == DefaultID)
            {
                out << i.thread_id << ": failed to determine student ID" << std::endl;
            }
            else if (i.id != form.key)
            {
                if (DEBUG)
                    out << std::left << std::setw(5) << i.thread_id;

                out << std::left << std::setw(10) << i.id << "\t";

                int same = 0;

                // Only score the ones that the teacher didn't leave blank
                for (size_type q = 0; q < i.answers.size(); ++q)
                {
                    if (key[q] != Answer::Blank)
                    {
                        out << i.answers[q] << " ";

                        if (key[q] == i.answers[q])
                            ++same;
                    }
                }

                out << std::endl;

                scores[i.id] = (total>0)?1.0*same/total:1;
            }
        }

        // Output scores
        out << std::endl << "Scores" << std::endl;

        for (const std::pair<long long, double>& score: scores)
        {
            out << "  " << std::left << std::setw(10) << score.first << " "
                << std::fixed << std::setprecision(2) << std::right << std::setw(6)
                << score.second*100 << "%" << std::endl;
        }
    }
    else
    {
        out << "Key not found. Given IDs:" << std::endl;

        for (const FormImage& i : form.formImages)
            if (i.id != DefaultID)
                out << "  " << i.id << std::endl;
    }

    if (!form.output.empty())
        out << std::endl << form.output;

    return out.str();
}

void Processor::add(long long id, long long key, const std::string& filename)
{
    std::unique_lock<std::mutex> lock(forms_mutex);
    forms.push_back(Form(id, key, filename, *this));
    extractT.queue(&forms.back());
}
