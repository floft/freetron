#include <cppcms/view.h>
#include <cppcms/form.h>
#include <string>
#include <regex>

#include "content.h"

const std::regex User::validUserRegex(R"(^[A-Za-z0-9\-_\.]+$)");

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

namespace content
{
    login_form::login_form()
    {
        user.message("Username");
        pass.message("Password");
        submit.value("Login");
        add(user);
        add(pass);
        add(submit);
        user.non_empty();
        pass.non_empty();
    }

    bool login_form::validate()
    {
        if (!form::validate())
            return false;

        User u(user.value(), pass.value());

        return u.valid();
    }

    newaccount_form::newaccount_form()
        : login_form()
    {
        user.message("New Username");
        pass.message("New Password");
        submit.value("Create");
    }

    updateaccount_form::updateaccount_form()
        : login_form()
    {
        user.message("Username");
        pass.message("Password");
        submit.value("Update");
    }
}
