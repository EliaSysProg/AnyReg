#pragma once

#include "RegistryEntry.hpp"
#include "RegistryRecordRange.hpp"

#include <filesystem>

namespace sql
{
    class DatabaseConnection;
    class ScopedTransaction;
}

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

    class InsertKeyStatement;

    class RegistryDatabase final
    {
    public:
        static RegistryDatabase create();
        static RegistryDatabase open_read();
        static RegistryDatabase open_write();

        ~RegistryDatabase();

        void save(const std::filesystem::path& filename) const;
        void load(const std::filesystem::path& filename);

        sql::ScopedTransaction* begin_transaction();
        void end_transaction(sql::ScopedTransaction* transaction);
        void insert_key(const RegistryKeyView& key);

        [[nodiscard]] size_t count_keys(std::string_view query) const;
        [[nodiscard]] sql::Statement* start_find_operation(SortColumn column, SortOrder order) const;
        void end_find_operation(sql::Statement* find_operation) const;
        void bind_find_operation(sql::Statement* statement, std::string_view query) const;
        void bind_find_operation(sql::Statement* statement, size_t offset, size_t count) const;
        void reset_find_operation(sql::Statement* statement) const;

    private:
        explicit RegistryDatabase(std::unique_ptr<sql::DatabaseConnection> db);

        std::unique_ptr<sql::DatabaseConnection> _db;
        std::unique_ptr<InsertKeyStatement> _insert_key_statement;
    };
}
