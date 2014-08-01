#include "processor.h"

void extractImages(Form* form)
{
    std::list<FormImage> newImages;

    try
    {
        // Get the images from the PDF while at the same time adding them to
        // the thread queue to process.
        newImages = extract(form->filename, form->processor.parseT, *form);

        // Set number of pages to however many images we got out of the PDF
        form->pages = newImages.size();
    }
    catch (const std::runtime_error& error)
    {
        form->log(form->filename + ", " + error.what());
        return;
    }
    catch (const PoDoFo::PdfError& error)
    {
        //error.PrintErrorMsg();
        //return error.GetError();

        // Don't write this to screen
        form->log(form->filename + ", " + error.what(), LogType::Error);
        return;
    }
    catch (...)
    {
        form->log("Unhandled exception");
        return;
    }

    // Move these new images to the actual list. Do this here instead
    // of directly appending them to the list so that we only need
    // to lock this once.
    std::lock_guard<std::mutex> lock(form->images_mutex);
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
}

void Processor::wait()
{
    extractT.wait();
    parseT.wait();
}

void Processor::exit()
{
    extractT.exit();
    parseT.exit();
}

Form& Processor::findForm(long long id)
{
    std::list<Form>::iterator i = std::find_if(forms.begin(), forms.end(),
            FormPredicate(id));

    if (i == forms.end())
        return defaultForm;
    else
        return *i;
}

std::string Processor::print(long long id)
{
    Form& form = findForm(id);
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
    std::lock_guard<std::mutex> lock(forms_mutex);
    forms.push_back(Form(id, key, filename, *this));
    extractT.queue(&forms.back());
}
