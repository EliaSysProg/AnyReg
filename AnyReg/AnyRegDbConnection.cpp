#include "Pch.hpp"
#include "AnyRegDbConnection.hpp"

#include "Registry.hpp"

#include <format>
#include <print>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

AnyRegDbConnection::AnyRegDbConnection(const std::string_view filename, const int flags)
    : _db(connect_database(filename, flags)),
      _insert_key_statement(
          _db,
          R"(INSERT INTO RegistryKeys (Key, LastWriteTime)
                VALUES (?1, ?2)
                ON CONFLICT(Key) DO UPDATE SET LastWriteTime = excluded.LastWriteTime
                    WHERE excluded.LastWriteTime > RegistryKeys.LastWriteTime;)"),
      _insert_value_statement(
          _db,
          R"(INSERT INTO RegistryValues (Value, Key, Type)
                VALUES (?1, ?2, ?3)
                ON CONFLICT(Value, Key) DO UPDATE SET Type = excluded.Type
                    WHERE excluded.Type != RegistryValues.Type;)")
{
    // Assuming indexing occurs as the lifetime of AnyRegDB.
    // TODO: Refactor to RegistryIndexer
    _db.execute("BEGIN TRANSACTION;");
}

AnyRegDbConnection::~AnyRegDbConnection()
{
    _db.execute("COMMIT;");
}

void AnyRegDbConnection::insert_key(const RegistryKeyEntry& key)
{
    _insert_key_statement.bind_text(1, key.path, true);
    _insert_key_statement.bind_int64(2, key.last_write_time.time_since_epoch().count());
    _insert_key_statement.step();
    _insert_key_statement.reset();
}

void AnyRegDbConnection::insert_value(const RegistryValueEntry& value)
{
    _insert_value_statement.bind_text(1, value.name, true);
    _insert_value_statement.bind_text(2, value.key, true);
    _insert_value_statement.bind_int(3, static_cast<int>(value.type));
    _insert_value_statement.step();
    _insert_value_statement.reset();
}

void AnyRegDbConnection::index(const HKEY root, std::string_view sub_path)
{
    std::vector<std::string> stack_keys;
    size_t keys_count = 0;
    size_t values_count = 0;
    stack_keys.reserve(0x1000);
    // TODO: Add to DB
    stack_keys.emplace_back(std::begin(sub_path), std::end(sub_path));

    RegistryKeyEntry key_entry;
    RegistryValueEntry value_entry;
    std::string temp_name;

    while (!stack_keys.empty())
    {
        if (keys_count % 10000 == 0)
        {
            std::println("Keys: {}", keys_count);
            std::println("Values: {}", values_count);
        }

        auto current_key = std::move(stack_keys.back());
        stack_keys.pop_back();

        if (!current_key.empty() && current_key.back() != '\\')
            current_key.push_back('\\');

        RegistryKey key;

        try
        {
            key = RegistryKey(root, current_key, KEY_READ);
        }
        catch (const std::system_error& e)
        {
            const auto error_code = e.code().value();
            if (error_code & (ERROR_ACCESS_DENIED | ERROR_PATH_NOT_FOUND))
            {
                continue;
            }
        }

        ++keys_count;

        // Enumerate values
        for (DWORD i = 0; key.get_value(i, value_entry.name, value_entry.type); ++i)
        {
            value_entry.key = current_key;
            insert_value(value_entry);
            ++values_count;
        }

        // Enumerate subkeys
        for (DWORD i = 0; key.get_sub_key(i, temp_name, key_entry.last_write_time); ++i)
        {
            key_entry.path = std::format("{}{}", current_key, temp_name);
            insert_key(key_entry);
            stack_keys.push_back(key_entry.path);
        }
    }
}

sql::DatabaseConnection AnyRegDbConnection::connect_database(const std::string_view filename, const int flags)
{
    sql::DatabaseConnection db(filename, flags);
    create_tables_if_does_not_exist(db);
    return db;
}

void AnyRegDbConnection::create_tables_if_does_not_exist(const sql::DatabaseConnection& connection)
{
    connection.execute(R"(
CREATE TABLE IF NOT EXISTS RegistryKeys (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Key TEXT NOT NULL UNIQUE,
    LastWriteTime INTEGER
);

    )");

    connection.execute(R"(
        CREATE TABLE IF NOT EXISTS RegistryValues (
            ID INTEGER PRIMARY KEY AUTOINCREMENT,
            Value TEXT NOT NULL,
            Key TEXT NOT NULL,
            Type INTEGER,
            UNIQUE(Value, Key),
            FOREIGN KEY(Key) REFERENCES RegistryKeys(Key) ON DELETE CASCADE
        );
    )");
}
