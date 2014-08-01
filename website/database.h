/*
 * Database access
 */

#ifndef H_DATABASE
#define H_DATABASE

#include <cppdb/frontend.h>
#include <iostream>
#include <sstream>
#include <string>

class Database
{
    cppdb::session db;

    // Queries
    cppdb::statement addUserQ;
    cppdb::statement validUserQ;
    cppdb::statement updateAccountQ;
    cppdb::statement deleteAccountQ;
    cppdb::statement initFileQ;

public:
    Database(const std::string& filename);

    // Return 0 if no user
    long long addUser(const std::string& user, const std::string& pass);
    long long validUser(const std::string& user, const std::string& pass);
    void updateAccount(const std::string& user, const std::string& pass,
        long long id);
    void deleteAccount(long long id);
    long long initFile(const std::string& name, long long userId,
        long long key);

private:
    void initialize();
};

#endif
