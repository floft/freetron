/*
 * The main website application
 */

#ifndef H_WEBSITE
#define H_WEBSITE

#include <cppcms/service.h>

#include "content.h"
#include "database.h"

class website : public cppcms::application
{
    Database db;

public:
    website(cppcms::service& srv);

    // Create menu and set template settings
    void init(content::master& c);

    // Site pages
    void home();
    void account();
    void forms();

    // Used when uploading a new form
    void upload();
    void process(std::string num);

    // 404 Page
    virtual void main(std::string url);

    // Log the user in, if valid
    bool login(const std::string& user, const std::string& pass);

    // Determine if logged in using session information
    bool loggedIn();
    bool loggedOut();
};

#endif
