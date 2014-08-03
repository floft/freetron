/*
 * Database access
 */

#ifndef H_DATABASE
#define H_DATABASE

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cppdb/frontend.h>

struct FormData
{
    long long id;
    long long key;
    long long userId;
    std::string name;
    std::string data;
    std::string date;
};

class Database
{
    bool initialized;
    cppdb::session db;

    // Queries
    cppdb::statement addUserQ;
    cppdb::statement validUserQ;
    cppdb::statement updateAccountQ;
    cppdb::statement deleteAccountQ;
    cppdb::statement initFormQ;
    cppdb::statement updateFormQ;
    cppdb::statement deleteFormQ;
    cppdb::statement getOneQ;
    cppdb::statement getAllQ;

public:
    Database();
    Database(const std::string& filename);

    // Return 0 if no user
    long long addUser(const std::string& user, const std::string& pass);
    long long validUser(const std::string& user, const std::string& pass);
    void updateAccount(const std::string& user, const std::string& pass,
        long long id);
    void deleteAccount(long long id);
    long long initForm(const std::string& name, long long userId,
        long long key, const std::string& date);
    void updateForm(long long id, const std::string& data);
    void deleteForm(long long userId, long long formId);

    // Specify ID if you want a single form, otherwise get all of
    // a users's forms
    std::vector<FormData> getForms(long long userId, long long id = 0);

private:
    void initialize();
};

#endif
