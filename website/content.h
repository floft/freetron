/*
 * Data for the website page templates
 */

#ifndef CONTENT_H
#define CONTENT_H

#include <cppcms/view.h>
#include <cppcms/form.h>
#include <string>
#include <regex>

struct MenuItem
{
    std::string path;
    std::string name;
    bool logon;

    MenuItem(const std::string& path, const std::string& name, bool logon = false)
        : path(path), name(name), logon(logon)
    {
    }
};

struct User
{
private:
    static const std::regex validUserRegex;

public:
    std::string user;
    std::string pass; // sha256sum

    User()
    {
    }

    User(const std::string& user, const std::string& pass)
        : user(user), pass(pass)
    {
    }

    bool valid();
};

namespace content
{
    struct login_form : public cppcms::form
    {
        cppcms::widgets::text user;
        cppcms::widgets::password pass;
        cppcms::widgets::submit submit;

        login_form();
        virtual bool validate();
    };

    struct newaccount_form : public login_form
    {
        newaccount_form();
    };

    struct updateaccount_form : public login_form
    {
        updateaccount_form();
    };

    struct master : public cppcms::base_content
    {
        std::string title;
        std::string pageName;
        std::vector<MenuItem> menuList;
        bool loggedIn;
    };

    struct account : public master
    {
        int confirm;
        std::string user;
        std::string page_content;

        /*login_form login;
        newaccount_form newaccount;
        updateaccount_form updateaccount;*/
    };

    struct forms : public master
    {
        std::list<std::string> news_list;
    };
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
