/*
 * The rpc application
 */

#ifndef H_RPC
#define H_RPC

#include <set>
#include <ctime>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <cppcms/service.h>
#include <cppcms/rpc_json.h>
#include <booster/aio/deadline_timer.h>

#include "database.h"
#include "../processor.h"

// Store both a request for a status update on a particular form and the ID of
// that form so we know which status update to give to which request
struct ProcessRequest
{
    time_t createTime;
    long long formId;
    booster::shared_ptr<cppcms::rpc::json_call> call;

    ProcessRequest(long long formId,
            booster::shared_ptr<cppcms::rpc::json_call> call)
        : createTime(time(NULL)), formId(formId), call(call)
    {
    }
};

class rpc : public cppcms::rpc::json_rpc_server
{
    Database& db;
    Processor& p;

    // Long polling requests
    std::mutex waiters_mutex;
    std::vector<ProcessRequest> waiters;

    // Timer for reseting really long requests
    booster::aio::deadline_timer timer;

    // Monitor processing statuses
    std::atomic_bool exiting;
    std::thread statusThread;

public:
    rpc(cppcms::service& srv, Database& db, Processor& p);
    ~rpc();

    void account_login(const std::string& user, const std::string& pass);
    void account_logout();
    void account_create(const std::string& user, const std::string& pass);
    void account_update(const std::string& user, const std::string& pass);
    void account_delete(int confirmation);
    void form_process(long long formId);
    void form_getone(long long formId);
    void form_getall();
    void form_delete(long long formId);
    void form_rename();

private:
    bool login(const std::string& user, const std::string& pass);
    void logout();
    bool loggedIn();
    bool loggedOut();
    void resetCode();
    void getForms(cppcms::json::value& v, long long formId = 0);

    // Remove really old long polling requests
    void on_timer(const booster::system::error_code& e);
    void remove_context(booster::shared_ptr<cppcms::rpc::json_call> call);

    // If this form is currently being waited on, then send the status update
    void broadcast(long long formId, int percentage);

    friend class StatusThread;
};

// Thread to monitor status updates, it blocks till Processor says another form
// has been processed and then calls broadcast() on each of the updates
class StatusThread
{
    rpc& parent;
    std::atomic_bool& exiting;

public:
    StatusThread(rpc& parent, std::atomic_bool& exiting)
        : parent(parent), exiting(exiting)
    {
    }

    void operator()();
};

// For search through the list of waiting requests
class RequestCallPredicate
{
    booster::shared_ptr<cppcms::rpc::json_call> call;

public:
    RequestCallPredicate(
            booster::shared_ptr<cppcms::rpc::json_call> call)
        : call(call)
    {
    }

    bool operator()(const ProcessRequest& r)
    {
        return r.call == call;
    }
};

class RequestIdPredicate
{
    long long id;

public:
    RequestIdPredicate(long long id)
        : id(id)
    {
    }

    bool operator()(const ProcessRequest& r)
    {
        return r.formId == id;
    }
};

#endif
