#pragma once

#include <filesystem>
#include <stop_token>
#include <string>
#include <span>

#include "FindKeyStatement.hpp"
#include "InsertKeyStatement.hpp"
#include "RegistryEntry.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

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

        class FindKeyRange
        {
        public:
            FindKeyRange() = default;
            explicit FindKeyRange(std::shared_ptr<FindKeyStatement> statement);
            [[nodiscard]] FindKeyStatement::iterator begin() const;
            [[nodiscard]] FindKeyStatement::iterator end() const;

        private:
            std::shared_ptr<FindKeyStatement> _statement = nullptr;
        };

        FindKeyRange find_keys(const std::string& query, const std::stop_token& stop_token = {});

        FindKeyRange find_keys(const std::string& query,
                               FindKeyStatement::SortColumn sort_column,
                               FindKeyStatement::SortOrder order = FindKeyStatement::SortOrder::ASCENDING,
                               const std::stop_token& stop_token = {});

        FindKeyRange find_keys(const std::string& query,
                               FindKeyStatement::SortColumn sort_column,
                               FindKeyStatement::SortOrder order,
                               size_t offset,
                               size_t limit) const;

        size_t get_key_count(const std::string& query,
                             FindKeyStatement::SortColumn sort_column,
                             FindKeyStatement::SortOrder order) const;

    private:
        static sql::DatabaseConnection connect(int flags);

        void index_hive(HKEY hive, const std::stop_token& stop_token = {});

        sql::DatabaseConnection _db;
        InsertKeyStatement _insert_key_statement;
    };
}
