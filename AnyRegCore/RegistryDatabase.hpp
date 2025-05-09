﻿#pragma once

#include <filesystem>
#include <stop_token>
#include <string>
#include <vector>

#include "FindKeyStatement.hpp"
#include "InsertKeyStatement.hpp"
#include "RegistryEntry.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

namespace anyreg
{
    class RegistryDatabase final
    {
    public:
        explicit RegistryDatabase(int flags = SQLITE_OPEN_READWRITE
            | SQLITE_OPEN_CREATE
            | SQLITE_OPEN_MEMORY);

        void index(const std::vector<HKEY>& hives, const std::stop_token& stop_token = {});
        void save(const std::filesystem::path& filename) const;
        void load(const std::filesystem::path& filename);

        void insert_key(const RegistryKeyEntry& key);
        void insert_value(const RegistryValueEntry& value);

        std::vector<RegistryKeyEntry> find_keys(const std::string& query, const std::stop_token& stop_token = {});

    private:
        static sql::DatabaseConnection connect(int flags);

        void index_hive(HKEY hive, const std::stop_token& stop_token = {});
        void index_sub_key(HKEY hive, const std::string& path,const std::stop_token& stop_token = {});

        sql::DatabaseConnection _db;
        InsertKeyStatement _insert_key_statement{_db};
        FindKeyStatement _find_key_statement{_db};
    };
}
