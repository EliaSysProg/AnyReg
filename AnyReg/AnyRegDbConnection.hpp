#pragma once

#include "RegistryEntry.hpp"
#include "SQLiteWrapper/DatabaseConnection.hpp"
#include "SQLiteWrapper/Statement.hpp"

class AnyRegDbConnection final
{
public:
    AnyRegDbConnection(std::string_view filename, int flags);
    ~AnyRegDbConnection();

    AnyRegDbConnection(AnyRegDbConnection&& other) noexcept = default;
    AnyRegDbConnection& operator=(AnyRegDbConnection&& other) noexcept = default;

    AnyRegDbConnection(const AnyRegDbConnection& other) = delete;
    AnyRegDbConnection& operator=(const AnyRegDbConnection& other) = delete;

    void insert_key(const RegistryKeyEntry& key);
    void insert_value(const RegistryValueEntry& value);

    void index(HKEY root, std::string_view sub_path = "");

private:
    static sql::DatabaseConnection connect_database(std::string_view filename, int flags);
    
    static void create_tables_if_does_not_exist(const sql::DatabaseConnection& connection);

private:
    sql::DatabaseConnection _db;
    sql::Statement _insert_key_statement;
    sql::Statement _insert_value_statement;
};
