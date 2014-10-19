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
    getPassQ = db << "select id, pass from users where user = ? limit 1";
    idExistsQ = db << "select id from users where id = ? limit 1";
    updateAccountQ = db << "update or ignore users set user = ?, pass = ? where id = ?";
    deleteAccountQ = db << "delete from users where id = ?";
    deleteUserFormsQ = db << "delete from forms where userId = ?";
    initFormQ = db << "insert into forms(name, userId, key, date) values(?, ?, ?, ?)";
    updateFormQ = db << "update or ignore forms set data = ?, csv = ? where id = ?";
    deleteFormQ = db << "delete from forms where id = ? and userId = ?";
    getOneQ = db << "select id, userId, key, name, data, date from forms where userId = ? and id = ? limit 1";
    getAllQ = db << "select id, userId, key, name, data, date from forms where userId = ?";
    getCsvQ = db << "select csv from forms where userId = ? and id = ? limit 1";
}

void Database::initialize()
{
    std::unique_lock<std::mutex> lck(lock);

    db << "create table if not exists users ("
              "id     integer primary key autoincrement not null,"
              "user   text unique not null,"
              "pass   text not null"
          ")"
       << cppdb::exec;

    db << "create table if not exists forms ("
              "id     integer primary key autoincrement not null,"
              "userId integer not null,"
              "key    integer not null,"
              "name   text not null,"
              "date   text not null,"
              "data   text,"
              "csv    text"
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

    initialized = true;
}

long long Database::addUser(const std::string& user, const std::string& pass)
{
    if (!initialized)
        return 0;

    std::unique_lock<std::mutex> lck(lock);

    addUserQ.bind(1, user);
    addUserQ.bind(2, pass);
    addUserQ.exec();

    long long id = addUserQ.last_insert_id();

    addUserQ.reset();

    return id;
}

std::pair<long long, std::string> Database::getPass(const std::string& user)
{
    if (!initialized)
        return std::pair<long long, std::string>(0, "");

    std::unique_lock<std::mutex> lck(lock);

    getPassQ.bind(1, user);

    cppdb::result r = getPassQ.row();

    long long id = 0;
    std::string pass;

    if (!r.empty())
    {
        id = r.get<long long>(0);
        pass = r.get<std::string>(1);
    }

    getPassQ.reset();

    return std::pair<long long, std::string>(id, pass);
}

bool Database::idExists(long long id)
{
    if (!initialized)
        return false;

    std::unique_lock<std::mutex> lck(lock);

    idExistsQ.bind(1, id);

    cppdb::result r = idExistsQ.row();
    long long foundId = (r.empty())?0:r.get<long long>(0);

    idExistsQ.reset();

    return foundId > 0;
}

bool Database::updateAccount(const std::string& user, const std::string& pass,
        long long id)
{
    if (!initialized)
        return false;

    std::unique_lock<std::mutex> lck(lock);

    updateAccountQ.bind(1, user);
    updateAccountQ.bind(2, pass);
    updateAccountQ.bind(3, id);
    updateAccountQ.exec();
    updateAccountQ.reset();
    unsigned long long affected = updateAccountQ.affected();

    return affected > 0;
}

bool Database::deleteAccount(long long id)
{
    if (!initialized)
        return false;

    std::unique_lock<std::mutex> lck(lock);

    // Delete user account
    deleteAccountQ.bind(1, id);
    deleteAccountQ.exec();
    deleteAccountQ.reset();
    unsigned long long affected = deleteAccountQ.affected();

    // Delete any user forms
    deleteUserFormsQ.bind(1, id);
    deleteUserFormsQ.exec();
    deleteUserFormsQ.reset();

    return affected > 0;
}

long long Database::initForm(const std::string& name, long long userId,
        long long key, const std::string& date)
{
    if (!initialized)
        return 0;

    std::unique_lock<std::mutex> lck(lock);

    initFormQ.bind(1, name);
    initFormQ.bind(2, userId);
    initFormQ.bind(3, key);
    initFormQ.bind(4, date);
    initFormQ.exec();

    long long id = initFormQ.last_insert_id();

    initFormQ.reset();

    return id;
}

bool Database::updateForm(long long id, const std::string& data, const std::string& csv)
{
    if (!initialized)
        return false;

    std::unique_lock<std::mutex> lck(lock);

    updateFormQ.bind(1, data);
    updateFormQ.bind(2, csv);
    updateFormQ.bind(3, id);
    updateFormQ.exec();
    unsigned long long affected = updateFormQ.affected();

    updateFormQ.reset();

    return affected > 0;
}

bool Database::deleteForm(long long userId, long long formId)
{
    if (!initialized)
        return false;

    std::unique_lock<std::mutex> lck(lock);

    deleteFormQ.bind(1, formId);
    deleteFormQ.bind(2, userId);
    deleteFormQ.exec();
    unsigned long long affected = deleteFormQ.affected();

    deleteFormQ.reset();

    return affected > 0;
}

std::vector<FormData> Database::getForms(long long userId, long long id)
{
    std::vector<FormData> forms;

    if (!initialized)
        return forms;

    std::unique_lock<std::mutex> lck(lock);
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

    return forms;
}

std::string Database::getCsv(long long userId, long long id)
{
    std::string csv;

    if (!initialized)
        return csv;

    std::unique_lock<std::mutex> lck(lock);

    getCsvQ.bind(1, userId);
    getCsvQ.bind(2, id);

    cppdb::result r = getCsvQ.row();

    if (!r.empty() && !r.is_null(0))
        csv = r.get<std::string>(0);

    getCsvQ.reset();

    return csv;
}
