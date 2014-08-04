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
    };

    struct forms : public master
    {
        std::string message;
    };
}

#endif
