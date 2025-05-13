#pragma once

#include "RegistryEntry.hpp"
#include "RegistryRecordRange.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

#include <filesystem>

namespace anyreg
{
    enum class SortColumn : uint8_t
    {
        NAME,
        PATH,
        LAST_WRITE_TIME,
    };

    enum class SortOrder : uint8_t
    {
        ASCENDING,
        DESCENDING,
    };

    class FindKeyStatement final
    {
    public:
        FindKeyStatement(const sql::DatabaseConnection& db, SortColumn column, SortOrder order);

        void bind(std::string_view query);
        void bind(size_t offset, size_t count);
        void reset();

        [[nodiscard]] RegistryRecordRange find();

    private:
        sql::Statement _statement;
    };

    class RegistryDatabase final
    {
    public:
        static RegistryDatabase create();
        static RegistryDatabase open_read();
        static RegistryDatabase open_write();

        void save(const std::filesystem::path& filename) const;
        void load(const std::filesystem::path& filename);

        [[nodiscard]] sql::ScopedTransaction begin_scoped_transaction() const;
        void insert_key(const RegistryKeyView& key);

        [[nodiscard]] size_t count_keys(std::string_view query) const;
        [[nodiscard]] FindKeyStatement find_keys(SortColumn column, SortOrder order) const;

    private:
        explicit RegistryDatabase(sql::DatabaseConnection db);

        sql::DatabaseConnection _db;
        sql::Statement _insert_key_statement;
    };
}
