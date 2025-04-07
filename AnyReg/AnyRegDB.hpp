#pragma once

#include "SQLiteWrapper/DatabaseConnection.hpp"
#include "SQLiteWrapper/Statement.hpp"

class AnyRegDB final
{
public:
    AnyRegDB(std::string_view filename, int flags);
    ~AnyRegDB();

    AnyRegDB(AnyRegDB&& other) noexcept = default;
    AnyRegDB& operator=(AnyRegDB&& other) noexcept = default;

    AnyRegDB(const AnyRegDB& other) = delete;
    AnyRegDB& operator=(const AnyRegDB& other) = delete;

    void insert_key(std::string_view key);
    void insert_value(std::string_view key, std::string_view value);

private:
    static sql::DatabaseConnection connect_database(std::string_view filename, int flags);
    
    static void create_tables_if_does_not_exist(sql::DatabaseConnection& connection);

private:
    sql::DatabaseConnection _db;
    sql::Statement _insert_key_statement;
    sql::Statement _insert_value_statement;
};
