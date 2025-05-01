#pragma once

#include "RegistryEntry.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

namespace anyreg
{
    class FindKeyStatement final
    {
    public:

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
        
        explicit FindKeyStatement(const sql::DatabaseConnection& db);

        void bind(const std::string& user_query, SortColumn sort_column = SortColumn::PATH, SortOrder order = SortOrder::ASCENDING);
        void reset_and_clear();

    public:
        class iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = RegistryKeyView;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            iterator() = default;
            explicit iterator(sql::Statement* statement);

            const RegistryKeyView& operator*() const;
            const RegistryKeyView* operator->() const;
            iterator& operator++();
            bool operator!=(const iterator& other) const;

        private:
            sql::Statement* _statement = nullptr;
            RegistryKeyView _current{};
        };

        iterator begin();
        iterator end();

    private:
        sql::Statement& current_statement();

        const sql::DatabaseConnection* _db;
        sql::Statement _statement;
        std::string _escaped_query;
        bool _has_value = false;
    };
}
