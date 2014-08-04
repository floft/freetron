/*
 * The rpc application
 */

#ifndef H_RPC
#define H_RPC

#include <cppcms/service.h>
#include <cppcms/rpc_json.h>

#include "database.h"
#include "../processor.h"

class rpc : public cppcms::rpc::json_rpc_server
{
    Database& db;
    Processor& p;

public:
    rpc(cppcms::service& srv, Database& db, Processor& p);

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
};

#endif