#include "forms.h"

bool operator==(const Form& a, const Form& b)
{
    return a.id == b.id;
}

bool operator!=(const Form& a, const Form& b)
{
    return a.id != b.id;
}

void Form::log(const std::string& msg, const LogType& t)
{
    std::ostringstream s;
    s << t << ": " << msg << std::endl;

    std::lock_guard<std::mutex> lock(output_mutex);
    output += s.str();
}

Form::Form(Form&& f)
    : id(f.id), key(f.key), pages(f.pages), processor(f.processor)
{
    filename = std::move(f.filename);
    formImages = std::move(f.formImages);

    std::lock_guard<std::mutex> lock(f.output_mutex);
    output = std::move(f.output);
}
