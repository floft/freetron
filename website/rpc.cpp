#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/mount_point.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "rpc.h"
#include "options.h"
#include "content.h"

rpc::rpc(cppcms::service& srv, Database& db, Processor& p)
    : cppcms::rpc::json_rpc_server(srv), db(db), p(p)
{
    // Random, used for form confirmations
    srand(time(NULL));

    // Account
    bind("account_login", cppcms::rpc::json_method(&rpc::account_login, this), method_role);
    bind("account_logout", cppcms::rpc::json_method(&rpc::account_logout, this), method_role);
    bind("account_create", cppcms::rpc::json_method(&rpc::account_create, this), method_role);
    bind("account_update", cppcms::rpc::json_method(&rpc::account_update, this), method_role);
    bind("account_delete", cppcms::rpc::json_method(&rpc::account_delete, this), method_role);

    // Forms
    bind("form_process", cppcms::rpc::json_method(&rpc::form_process, this), method_role);
    bind("form_getone", cppcms::rpc::json_method(&rpc::form_getone, this), method_role);
    bind("form_getall", cppcms::rpc::json_method(&rpc::form_getall, this), method_role);
    bind("form_delete", cppcms::rpc::json_method(&rpc::form_delete, this), method_role);
    bind("form_rename", cppcms::rpc::json_method(&rpc::form_rename, this), method_role);
}

void rpc::account_login(const std::string& user, const std::string& pass)
{
    if (loggedOut() && !user.empty() && !pass.empty() && session().is_set("prelogin"))
    {
        User u(user, pass);

        if (u.valid() && login(user, pass))
        {
            return_result(true);
            return;
        }
    }

    return_result(false);
}

void rpc::account_logout()
{
    logout();
    return_result(true);
}

void rpc::account_create(const std::string& user, const std::string& pass)
{
    if (loggedOut() && !user.empty() && !pass.empty())
    {
        User u(user, pass);

        if (u.valid() && db.addUser(user, pass) > 0 && login(user, pass))
        {
            return_result(true);
            return;
        }
    }

    return_result(false);
}

void rpc::account_update(const std::string& user, const std::string& pass)
{
    if (loggedIn() && !user.empty() && !pass.empty())
    {
        User u(user, pass);
        long long id = session().get<long long>("id");

        if (u.valid() && db.updateAccount(user, pass, id))
        {
            session().set<std::string>("user", user);

            return_result(true);
            return;
        }
    }

    return_result(false);
}

void rpc::account_delete(int confirmation)
{
    if (loggedIn() && confirmation == session().get<int>("confirmation"))
    {
        long long id = session().get<long long>("id");

        if (db.deleteAccount(id))
        {
            logout();

            return_result(true);
            return;
        }
    }

    return_result(false);
}

void rpc::form_process(long long formId)
{
    cppcms::json::value v = cppcms::json::object();
    cppcms::json::object& obj = v.object();

    // Send the ID again so we can easily create the next request
    obj["id"] = formId;
    obj["percent"] = 0;

    if (loggedIn())
        obj["percent"] = (p.done(formId))?100:p.statusWait(formId);

    return_result(v);
}

void rpc::form_getone(long long formId)
{
    cppcms::json::value v = cppcms::json::array();

    if (loggedIn())
        getForms(v, formId);

    return_result(v);
}

void rpc::form_getall()
{
    cppcms::json::value v = cppcms::json::array();

    if (loggedIn())
        getForms(v);

    return_result(v);
}

void rpc::getForms(cppcms::json::value& v, long long formId)
{
    long long userId = session().get<long long>("id");

    // If formId == 0, then it'll get all the forms
    std::vector<FormData> forms = db.getForms(userId, formId);

    // Optimization
    cppcms::json::array& ar = v.array();
    ar.reserve(forms.size());

    for (const FormData& f : forms)
    {
        cppcms::json::object obj;
        obj["id"] = f.id;
        obj["key"] = f.key;
        obj["name"] = f.name;
        obj["data"] = f.data;
        obj["date"] = f.date;
        ar.push_back(obj);
    }
}

void rpc::form_delete(long long formId)
{
    if (loggedIn())
    {
        long long userId = session().get<long long>("id");

        if (db.deleteForm(userId, formId))
        {
            return_result(true);
            return;
        }
    }

    return_result(false);
}

void rpc::form_rename()
{
    if (loggedIn())
    {
    }

    return_result(false);
}

bool rpc::loggedIn()
{
    if (session().is_set("loggedIn") && session().get<bool>("loggedIn"))
        return true;

    return false;
}

bool rpc::loggedOut()
{
    return !loggedIn();
}

bool rpc::login(const std::string& user, const std::string& pass)
{
    // See if valid user
    long long id = db.validUser(user, pass);

    if (id > 0)
    {
        session().reset_session();
        session().erase("prelogin");
        session().set<bool>("loggedIn", true);
        session().set<long long>("id", id);
        session().set<std::string>("user", user);
        resetCode();

        return true;
    }

    return false;
}

void rpc::logout()
{
    if (loggedIn())
    {
        session().set<bool>("loggedIn", false);

        session().erase("id");
        session().erase("user");
        session().erase("loggedIn");
    }

    session().reset_session();
}

void rpc::resetCode()
{
    session().set<int>("confirmation", rand());
}
