/*
 * The main website application
 */

#ifndef H_WEBSITE
#define H_WEBSITE

#include <string>
#include <cppcms/service.h>

#include "date.h"
#include "content.h"
#include "database.h"
#include "../processor.h"

// We can only pass in two arguments to an CppCMS application
struct WebsiteData
{
    Database& db;
    Processor& p;
    long long maxFilesize;
    std::string root;

    WebsiteData(Database& db, Processor& p,
        long long maxFilesize, std::string root = "")
        : db(db), p(p), maxFilesize(maxFilesize), root(root)
    {
    }
};

class website : public cppcms::application
{
    Date date;
    Database& db;
    Processor& p;
    long long maxFilesize;

public:
    // Specify root if this is in a subdirectory, e.g. /website
    website(cppcms::service& srv, WebsiteData d);

    // Create menu and set template settings
    void initTemplate(content::master& c);

    // Site pages
    void home();
    void account();
    void forms();

    // Used on the forms page to submit the uploaded file via Javascript
    void upload(std::string num);
    long long uploadFile(long long key);

    // 404 Page
    virtual void main(std::string url);

    // Log the user in, if valid
    bool login(const std::string& user, const std::string& pass);

    // Determine if logged in using session information
    bool loggedIn();
    bool loggedOut();
};

#endif
