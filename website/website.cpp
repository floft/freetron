#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_file.h>
#include <algorithm>
#include <iostream>
#include <unistd.h>

#include "website.h"
#include "options.h"

website::website(cppcms::service& srv, Database& db, Processor& p)
    : cppcms::application(srv), db(db), p(p)
{
    dispatcher().assign("", &website::home, this);
    mapper().assign("");

    dispatcher().assign("/forms", &website::forms, this);
    mapper().assign("forms", "/forms");

    dispatcher().assign("/account", &website::account, this);
    mapper().assign("account", "/account");

    dispatcher().assign("/upload/(\\d+)", &website::upload, this, 1);
    mapper().assign("upload", "/upload/{1}");

    dispatcher().assign("/process/(\\d+)", &website::process, this, 1);
    mapper().assign("process", "/process/{1}");

    //mapper().root("/website");
}

void website::init(content::master& c)
{
    c.title = "Freetron";
    c.loggedIn = loggedIn();
    c.menuList.push_back(MenuItem("/", "Home"));
    c.menuList.push_back(MenuItem("/forms", "Forms", true));
    c.menuList.push_back(MenuItem("/account", "Account"));
}

void website::home()
{
    content::master c;
    init(c);
    c.pageName = "Home";
    render("home", c);
}

void website::account()
{
    content::account c;
    init(c);
    c.user = "";
    c.confirm = 0;
    c.pageName = "Account";

    if (loggedIn())
    {
        c.user = session().get<std::string>("user");
        c.confirm = session().get<int>("confirmation");
    }
    else
    {
        session().set<bool>("prelogin", true);
    }

    render("account", c);
}

void website::forms()
{
    content::forms c;
    init(c);
    c.pageName = "Forms";
    //c.news_list.push_back("This is a test message");
    render("forms", c);
}

void website::upload(std::string num)
{
    response().set_plain_text_header();

    if (!loggedIn())
        return;

    long long key = atoll(num.c_str());
    long long userId = session().get<long long>("id");

    if (request().request_method() == "POST")
    {
        for (booster::shared_ptr<cppcms::http::file> file : request().files())
        {
            // Maybe we should move this
            static const long long maxFilesize = 250*1024*1024; // 250 MB

            // Get lowercase last three letters, the extension, which should be "pdf"
            if (file->filename().length() <= 3)
                continue;

            std::string ext = file->filename().substr(file->filename().length() - 3);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            if (file->name() == "file" && (file->mime() == "application/pdf" ||
                ext == "pdf") && file->size() < maxFilesize)
            {
                std::string name = file->filename();

                // Add to database
                long long id = db.initFile(name, userId, key);

                // Save to disk
                std::ostringstream s;
                s << "./uploads/" << id << ".pdf";
                file->save_to(s.str());

                // Start processing
                p.add(id, key, s.str());

                response().out() << id;
                return;
            }
        }
    }

    response().out() << "failed";
}

void website::process(std::string num)
{
    response().io_mode(cppcms::http::response::nogzip);
    response().set_plain_text_header();

    if (!loggedIn())
        return;

    long long id = atoll(num.c_str());

    if (p.done(id))
    {
        response().out() << 100 << "\n";
        response().out() << id << "\n";
        response().out() << std::flush;
    }
    else
    {
        int percent = 0;
        response().out() << 0 << "\n";
        response().out() << std::flush;

        while (percent != 100)
        {
            percent = p.statusWait(id);
            response().out() << percent << "\n";
            response().out() << std::flush;
        }
    }

    //response().out() << "failed" << "\n";
}

// 404 Page
void website::main(std::string url)
{
    if (!dispatcher().dispatch(url))
    {
        response().status(cppcms::http::response::not_found);
        content::master c;
        init(c);
        c.pageName = "404 Not Found";
        render("notfound", c);
    }
}

bool website::loggedIn()
{
    if (session().is_set("loggedIn") && session().get<bool>("loggedIn"))
        return true;

    return false;
}

bool website::loggedOut()
{
    return !loggedIn();
}
