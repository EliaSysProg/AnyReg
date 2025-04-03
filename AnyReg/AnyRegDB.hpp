#pragma once

#include "SQLiteWrapper/DatabaseConnection.hpp"

class AnyRegDB final
{
public:
    explicit AnyRegDB(std::string filename);
    ~AnyRegDB() = default;

    AnyRegDB(AnyRegDB&& other) noexcept = default;
    AnyRegDB& operator=(AnyRegDB&& other) noexcept = default;

    AnyRegDB(const AnyRegDB& other) = delete;
    AnyRegDB& operator=(const AnyRegDB& other) = delete;

    void insert_key(std::string_view key);
    void insert_value(std::string_view key, std::string_view value);

private:
    sql::DatabaseConnection _db;
};
