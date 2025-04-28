#pragma once

#include "RegistryEntry.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

namespace anyreg
{
    class FindKeyStatement final
    {
    public:
        explicit FindKeyStatement(const sql::DatabaseConnection& db);

        void bind(const std::string& user_query);
        // [[nodiscard]] bool has_value() const;
        // RegistryKeyEntry get_value();
        // void step();
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
        
        sql::Statement _fts_statement;
        sql::Statement _like_statement;
        std::string _escaped_query;
        bool _is_fts = false;
        bool _has_value = false;
    };
}
