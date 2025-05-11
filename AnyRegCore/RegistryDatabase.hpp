#pragma once

#include "EmptyStatement.hpp"
#include "FtsStatement.hpp"
#include "InsertKeyStatement.hpp"
#include "LikeStatement.hpp"
#include "RegistryEntry.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

#include <filesystem>
#include <span>
#include <stop_token>
#include <string>

namespace anyreg
{
    class RegistryDatabase final
    {
    public:
        explicit RegistryDatabase(int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

        void index(std::span<const HKEY> hives, const std::stop_token& stop_token = {});
        void save(const std::filesystem::path& filename) const;
        void load(const std::filesystem::path& filename);

        void insert_key(const RegistryKeyEntry& key);
        void insert_value(const RegistryValueEntry& value);

        [[nodiscard]] EmptyStatement get_empty_query(SortColumn sort_column, SortOrder sort_order) const;
        [[nodiscard]] LikeStatement get_like_query(SortColumn sort_column, SortOrder sort_order) const;
        [[nodiscard]] FtsStatement get_fts_query(SortColumn sort_column, SortOrder sort_order) const;

    private:
        static sql::DatabaseConnection connect(int flags);

        void index_hive(HKEY hive, const std::stop_token& stop_token = {});

        sql::DatabaseConnection _db;
        InsertKeyStatement _insert_key_statement;
    };
}
