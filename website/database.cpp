#include "database.h"

Database::Database(const std::string& filename)
{
    std::ostringstream s;
    s << "sqlite3:db=" << filename;
    db = cppdb::session(s.str());

    initialize();

    addUserQ = db << "insert or ignore into users(user, pass) values(?, ?)";
    validUserQ = db << "select id from users where user = ? and pass = ? limit 1";
    updateAccountQ = db << "update or ignore users set user = ?, pass = ? where id = ?";
    deleteAccountQ = db << "delete from users where id = ?";
    initFileQ = db << "insert into forms(name, userId, key) values(?, ?, ?)";
}

void Database::initialize()
{
    cppdb::transaction guard(db);

    db << "create table if not exists users ("
              "id     integer primary key autoincrement not null,"
              "user   text unique not null,"
              "pass   text not null" // a hash
          ")"
       << cppdb::exec;

    db << "create table if not exists forms ("
              "id     integer primary key autoincrement not null,"
              "userId integer not null,"
              "key    integer not null,"
              "name   text not null,"
              "data   text"
          ")"
       << cppdb::exec;

    // Create dummy user if it doesn't already exist to start the autoincrement
    // at one so that we can just check for zero return values in the insert
    // functions later on
    //
    // Note: a space isn't a valid username anyway, so it won't conflict with
    // any somebody would actually try to create. And, the password isn't a
    // proper hash, so it won't match either.
    db << R"(insert or ignore into users(id, user, pass) values(0, " ", "$"))"
       << cppdb::exec;

    guard.commit();
}

long long Database::addUser(const std::string& user, const std::string& pass)
{
    cppdb::transaction guard(db);

    addUserQ.bind(1, user);
    addUserQ.bind(2, pass);
    addUserQ.exec();

    long long id = addUserQ.last_insert_id();

    addUserQ.reset();
    guard.commit();

    return id;
}

long long Database::validUser(const std::string& user, const std::string& pass)
{
    cppdb::transaction guard(db);

    validUserQ.bind(1, user);
    validUserQ.bind(2, pass);

    long long id;
    cppdb::result r = validUserQ.row();

    if (r.empty())
        id = 0;
    else
        id = r.get<long long>(0);

    validUserQ.reset();
    guard.commit();

    return id;
}

void Database::updateAccount(const std::string& user, const std::string& pass,
        long long id)
{
    cppdb::transaction guard(db);

    updateAccountQ.bind(1, user);
    updateAccountQ.bind(2, pass);
    updateAccountQ.bind(3, id);
    updateAccountQ.exec();
    updateAccountQ.reset();

    guard.commit();
}

void Database::deleteAccount(long long id)
{
    cppdb::transaction guard(db);

    deleteAccountQ.bind(1, id);
    deleteAccountQ.exec();
    deleteAccountQ.reset();

    guard.commit();
}

long long Database::initFile(const std::string& name, long long userId, long long key)
{
    cppdb::transaction guard(db);

    initFileQ.bind(1, name);
    initFileQ.bind(2, userId);
    initFileQ.bind(3, key);
    initFileQ.exec();

    long long id = initFileQ.last_insert_id();

    initFileQ.reset();
    guard.commit();

    return id;
}
