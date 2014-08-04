#include "database.h"

Database::Database()
    : initialized(false)
{
}

Database::Database(const std::string& filename)
    : initialized(false)
{
    std::ostringstream s;
    s << "sqlite3:db=" << filename;
    db = cppdb::session(s.str());

    initialize();

    addUserQ = db << "insert or ignore into users(user, pass) values(?, ?)";
    validUserQ = db << "select id from users where user = ? and pass = ? limit 1";
    idExistsQ = db << "select id from users where id = ? limit 1";
    updateAccountQ = db << "update or ignore users set user = ?, pass = ? where id = ?";
    deleteAccountQ = db << "delete from users where id = ?";
    deleteUserFormsQ = db << "delete from forms where userId = ?";
    initFormQ = db << "insert into forms(name, userId, key, date) values(?, ?, ?, ?)";
    updateFormQ = db << "update or ignore forms set data = ? where id = ?";
    deleteFormQ = db << "delete from forms where id = ? and userId = ?";
    getOneQ = db << "select id, userId, key, name, data, date from forms where userId = ? and id = ? limit 1";
    getAllQ = db << "select id, userId, key, name, data, date from forms where userId = ?";
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
              "date   text not null,"
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

    initialized = true;
}

long long Database::addUser(const std::string& user, const std::string& pass)
{
    if (!initialized)
        return 0;

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
    if (!initialized)
        return 0;

    cppdb::transaction guard(db);

    validUserQ.bind(1, user);
    validUserQ.bind(2, pass);

    cppdb::result r = validUserQ.row();
    long long id = (r.empty())?0:r.get<long long>(0);

    validUserQ.reset();
    guard.commit();

    return id;
}

bool Database::idExists(long long id)
{
    if (!initialized)
        return false;

    cppdb::transaction guard(db);

    idExistsQ.bind(1, id);

    cppdb::result r = idExistsQ.row();
    long long foundId = (r.empty())?0:r.get<long long>(0);

    idExistsQ.reset();
    guard.commit();

    return foundId > 0;
}


bool Database::updateAccount(const std::string& user, const std::string& pass,
        long long id)
{
    if (!initialized)
        return false;

    cppdb::transaction guard(db);

    updateAccountQ.bind(1, user);
    updateAccountQ.bind(2, pass);
    updateAccountQ.bind(3, id);
    updateAccountQ.exec();
    updateAccountQ.reset();
    unsigned long long affected = updateAccountQ.affected();

    guard.commit();

    return affected > 0;
}

bool Database::deleteAccount(long long id)
{
    if (!initialized)
        return false;

    cppdb::transaction guard(db);

    // Delete user account
    deleteAccountQ.bind(1, id);
    deleteAccountQ.exec();
    deleteAccountQ.reset();
    unsigned long long affected = deleteAccountQ.affected();

    // Delete any user forms
    deleteUserFormsQ.bind(1, id);
    deleteUserFormsQ.exec();
    deleteUserFormsQ.reset();

    guard.commit();

    return affected > 0;
}

long long Database::initForm(const std::string& name, long long userId,
        long long key, const std::string& date)
{
    if (!initialized)
        return 0;

    cppdb::transaction guard(db);

    initFormQ.bind(1, name);
    initFormQ.bind(2, userId);
    initFormQ.bind(3, key);
    initFormQ.bind(4, date);
    initFormQ.exec();

    long long id = initFormQ.last_insert_id();

    initFormQ.reset();
    guard.commit();

    return id;
}

bool Database::updateForm(long long id, const std::string& data)
{
    if (!initialized)
        return false;

    cppdb::transaction guard(db);

    updateFormQ.bind(1, data);
    updateFormQ.bind(2, id);
    updateFormQ.exec();
    unsigned long long affected = updateFormQ.affected();

    updateFormQ.reset();
    guard.commit();

    return affected > 0;
}

bool Database::deleteForm(long long userId, long long formId)
{
    if (!initialized)
        return false;

    cppdb::transaction guard(db);

    deleteFormQ.bind(1, formId);
    deleteFormQ.bind(2, userId);
    deleteFormQ.exec();
    unsigned long long affected = deleteFormQ.affected();

    deleteFormQ.reset();
    guard.commit();

    return affected > 0;
}

std::vector<FormData> Database::getForms(long long userId, long long id)
{
    std::vector<FormData> forms;

    if (!initialized)
        return forms;

    cppdb::transaction guard(db);
    cppdb::result r;

    // Only one
    if (id != 0)
    {
        getOneQ.bind(1, userId);
        getOneQ.bind(2, id);
        r = getOneQ.query();
    }
    // All of them
    else
    {
        getAllQ.bind(1, userId);
        r = getAllQ.query();
    }

    FormData d;

    while (r.next())
    {
        r.fetch(0, d.id);
        r.fetch(1, d.userId);
        r.fetch(2, d.key);
        r.fetch(3, d.name);
        r.fetch(4, d.data);
        r.fetch(5, d.date);
        forms.push_back(d);
    }

    getOneQ.reset();
    getAllQ.reset();
    guard.commit();

    return forms;
}
