#pragma once

#include "FindKeyStatement.hpp"
#include "RegistryEntry.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

#include <filesystem>

namespace anyreg
{
    class RegistryDatabase final
    {
    public:
        static RegistryDatabase create();
        static RegistryDatabase open_read();
        static RegistryDatabase open_write();

        void save(const std::filesystem::path& filename) const;
        void load(const std::filesystem::path& filename);

        [[nodiscard]] sql::ScopedTransaction begin_scoped_transaction() const;
        [[nodiscard]] sql::ProgressHandler create_progress_handler(std::function<bool()> callback) const;
        int64_t insert_key(const RegistryKeyView& key);
        [[nodiscard]] RegistryKeyView get_key(int64_t id) const;

        [[nodiscard]] size_t count_keys(std::string_view query) const;
        [[nodiscard]] FindKeyStatement find_keys(SortColumn column, SortOrder order) const;
        [[nodiscard]] int64_t last_insert_rowid() const;

        [[nodiscard]] const sql::DatabaseConnection& get() const;

    private:
        explicit RegistryDatabase(sql::DatabaseConnection db);

        sql::DatabaseConnection _db;
        sql::Statement _insert_key_statement;
        mutable sql::Statement _get_key_statement;
    };
}
