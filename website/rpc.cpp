#include <vector>
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <cppcms/application.h>
#include <cppcms/mount_point.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/applications_pool.h>
#include <booster/system_error.h>
#include <booster/intrusive_ptr.h>
#include <boost/bind.hpp>
#include <openssl/sha.h>

#include "rpc.h"
#include "content.h"

rpc::rpc(cppcms::service& srv, Database& db, Processor& p)
    : cppcms::rpc::json_rpc_server(srv), db(db), p(p),
      timer(srv.get_io_service()), exiting(false),
      statusThread(StatusThread(*this, exiting))
{
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

    // Timeouts for getting rid of long requests
    on_timer(booster::system::error_code());
}

rpc::~rpc()
{
    timer.reset_io_service();

    // We need to tell Processor to wake up all the statusWait() calls we're
    // waiting on for status updates so we can exit that thread
    exiting = true;
    p.statusWakeAll();
    statusThread.join();
}

void rpc::account_login(const std::string& user, const std::string& pass)
{
    session().load();

    if (loggedIn())
        logout();

    if (!user.empty() && !pass.empty() && session().is_set("prelogin"))
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
    session().load();

    logout();
    return_result(true);
}

void rpc::account_create(const std::string& user, const std::string& inputPass)
{
    session().load();

    if (loggedOut() && !user.empty() && !inputPass.empty())
    {
        User u(user, inputPass);
        std::string hash = genPass(inputPass);

        if (u.valid() && db.addUser(user, hash) > 0 && login(user, inputPass))
        {
            return_result(true);
            return;
        }
    }

    return_result(false);
}

void rpc::account_update(const std::string& user, const std::string& inputPass)
{
    session().load();

    if (loggedIn() && !user.empty() && !inputPass.empty())
    {
        User u(user, inputPass);
        std::string hash = genPass(inputPass);
        long long id = session().get<long long>("id");

        if (u.valid() && db.updateAccount(user, hash, id))
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
    session().load();

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
    session().load();

    if (loggedIn())
    {
        if (p.done(formId))
        {
            cppcms::json::value v = cppcms::json::object();
            cppcms::json::object& obj = v.object();

            // Send the ID again so we can easily create the next request
            obj["id"] = formId;
            obj["percent"] = 100;

            return_result(v);
        }
        else
        {
            // If it's not done, then save this request and process it once we
            // are notified that this form is done, or if a timeout occurs
            std::unique_lock<std::mutex> lock(waiters_mutex);

            std::vector<ProcessRequest>::iterator i = std::find_if(
                    waiters.begin(), waiters.end(), RequestIdPredicate(formId));

            // Only save the request if it's the first one, otherwise somebody
            // could create massive amounts of requests that hang around for up
            // to the timeout using up server memory
            if (i == waiters.end())
            {
                booster::shared_ptr<cppcms::rpc::json_call> call = release_call();
                waiters.push_back(ProcessRequest(formId, call));

                // If the connection is closed before it's done, remove the request
                call->context().async_on_peer_reset(
                        boost::bind(
                            &rpc::remove_context,
                            booster::intrusive_ptr<rpc>(this),
                            call));
            }
            else
            {
                return_error("too many connections to this form");
            }
        }
    }
    else
    {
        return_error("not logged in");
    }
}

void rpc::form_getone(long long formId)
{
    session().load();

    cppcms::json::value v = cppcms::json::array();

    if (loggedIn())
        getForms(v, formId);

    return_result(v);
}

void rpc::form_getall()
{
    session().load();

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
        obj["data"] = (f.data.empty())?"Processing... please wait":f.data;
        obj["date"] = f.date;
        ar.push_back(obj);
    }
}

void rpc::form_delete(long long formId)
{
    session().load();

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
    session().load();

    if (loggedIn())
    {
    }

    return_result(false);
}

bool rpc::loggedIn()
{
    if (session().is_set("loggedIn") && session().get<bool>("loggedIn") &&
        db.idExists(session().get<long long>("id")))
        return true;

    return false;
}

bool rpc::loggedOut()
{
    return !loggedIn();
}

bool rpc::login(const std::string& user, const std::string& inputPass)
{
    // See if valid user
    std::pair<long long, std::string> results = db.getPass(user);
    const long long& id = results.first;
    const std::string& correctPass = results.second;

    if (id > 0 && passCorrect(correctPass, inputPass))
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

void rpc::on_timer(const booster::system::error_code& e)
{
    // Cancel
    if (e)
        return;

    // In seconds
    int timeout = 300; // 5 min

    // Remove really old connections
    std::unique_lock<std::mutex> lock(waiters_mutex);

    for (std::vector<ProcessRequest>::iterator i = waiters.begin();
            i != waiters.end(); ++i)
    {
        if (time(NULL) - i->createTime > timeout)
        {
            i->call->return_error("Connection closed by server");
            i = waiters.erase(i);

            if (i == waiters.end())
                break;
        }
    }

    // Restart timer
    timer.expires_from_now(booster::ptime::seconds(30));
    timer.async_wait(boost::bind(&rpc::on_timer, booster::intrusive_ptr<rpc>(this), _1));
}

void rpc::remove_context(booster::shared_ptr<cppcms::rpc::json_call> call)
{
    std::unique_lock<std::mutex> lock(waiters_mutex);
    std::vector<ProcessRequest>::iterator i = std::find_if(
            waiters.begin(), waiters.end(), RequestCallPredicate(call));

    if (i != waiters.end())
        waiters.erase(i);
}

void rpc::broadcast(long long formId, int percentage)
{
    cppcms::json::value v = cppcms::json::object();
    cppcms::json::object& obj = v.object();

    // Send the ID again so we can easily create the next request
    obj["id"] = formId;
    obj["percent"] = percentage;

    std::unique_lock<std::mutex> lock(waiters_mutex);

    for (std::vector<ProcessRequest>::iterator i = waiters.begin();
            i != waiters.end(); ++i)
    {
        if (i->formId == formId)
        {
            i->call->return_result(v);

            // Remove this under the assumption that there is only one request
            // per formId
            waiters.erase(i);
            break;
        }
    }
}

void StatusThread::operator()()
{
    while (!exiting)
    {
        std::vector<Status> results = parent.p.statusWait();

        if (exiting)
            break;

        for (Status& s : results)
            parent.broadcast(s.formId, s.percentage);
    }
}

// See: http://stackoverflow.com/q/13784434/2698494
std::string rpc::sha256(const std::string& s) const
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, s.c_str(), s.size());
    SHA256_Final(hash, &sha256);

    std::ostringstream ss;

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return ss.str();
}

std::string rpc::genSalt()
{
    int size = 32;
    std::ostringstream ss;
    std::vector<unsigned char> salt(size, 0);
    urand.generate(&salt[0], size);

    for (int i = 0; i < size; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];

    return ss.str();
}

// See: http://stackoverflow.com/a/236803/2698494
std::vector<std::string> rpc::split(const std::string& s, char delim) const
{
    std::string item;
    std::istringstream ss(s);
    std::vector<std::string> out;

    while (std::getline(ss, item, delim))
        out.push_back(item);

    return out;
}

// See: https://crackstation.net/hashing-security.htm#slowequals
bool rpc::slowCmp(const std::string& a, const std::string& b) const
{
    const char* A = a.c_str();
    const char* B = b.c_str();

    // Lengths are the same
    int diff = a.size() ^ b.size();

    // Bytes are the same
    for (unsigned int i = 0; i < a.size() && i < b.size(); ++i)
        diff |= A[i] ^ B[i];

    return diff == 0;
}

std::string rpc::genPass(const std::string& pass)
{
    std::string salt = genSalt();
    return salt + ":" + sha256(salt + pass);
}

bool rpc::passCorrect(const std::string& correctPass,
        const std::string& inputPass) const
{
    std::vector<std::string> parts = split(correctPass, ':');

    if (parts.size() != 2)
        return false;

    const std::string& salt = parts[0];
    const std::string& pass = parts[1];

    return slowCmp(sha256(salt + inputPass), pass);
}
