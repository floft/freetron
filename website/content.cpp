#include <regex>
#include <string>
#include <cppcms/view.h>
#include <cppcms/form.h>

#include "content.h"

const std::regex User::validUserRegex("^[A-Za-z0-9\\-_\\.]+$");

bool User::valid()
{
    // Must be sha256sum
    if (pass.length() != 64)
        return false;

    if (user.length() < 4 || user.length() > 30)
        return false;

    if (!std::regex_match(user, validUserRegex))
        return false;

    return true;
}
