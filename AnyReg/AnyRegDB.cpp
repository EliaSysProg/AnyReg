#include "Pch.hpp"
#include "AnyRegDB.hpp"

AnyRegDB::AnyRegDB(const std::string_view filename, const int flags)
    : _db(connect_database(filename, flags)),
      _insert_key_statement(
          _db,
          R"(INSERT INTO RegistryKeys (Key) VALUES (?1) ON CONFLICT(Key) DO NOTHING;)"),
      _insert_value_statement(
          _db,
          R"(INSERT INTO RegistryValues (Key, Value) VALUES (?1, ?2) ON CONFLICT(Key, Value) DO NOTHING;)")
{
    _db.execute("BEGIN TRANSACTION;");
}

AnyRegDB::~AnyRegDB()
{
    _db.execute("COMMIT;");
}

void AnyRegDB::insert_key(const std::string_view key)
{
    _insert_key_statement.bind_text(1, key, true);
    _insert_key_statement.step();
    _insert_key_statement.reset();
}

void AnyRegDB::insert_value(const std::string_view key, const std::string_view value)
{
    _insert_value_statement.bind_text(1, key, true);
    _insert_value_statement.bind_text(2, value, true);
    _insert_value_statement.step();
    _insert_value_statement.reset();
}

sql::DatabaseConnection AnyRegDB::connect_database(std::string_view filename, int flags)
{
    sql::DatabaseConnection db(filename, flags);
    create_tables_if_does_not_exist(db);
    return db;
}

void AnyRegDB::create_tables_if_does_not_exist(sql::DatabaseConnection& db)
{
    db.execute(R"(
        CREATE TABLE IF NOT EXISTS RegistryKeys (
            ID INTEGER PRIMARY KEY AUTOINCREMENT,
            Key TEXT NOT NULL UNIQUE
        );
    )");

    db.execute(R"(
        CREATE TABLE IF NOT EXISTS RegistryValues (
            ID INTEGER PRIMARY KEY AUTOINCREMENT,
            Key TEXT NOT NULL,
            Value TEXT NOT NULL,
            UNIQUE(Key, Value),
            FOREIGN KEY(Key) REFERENCES RegistryKeys(Key) ON DELETE CASCADE
        );
    )");
}
