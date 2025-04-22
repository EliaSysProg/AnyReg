#pragma once

#include "RegistryEntry.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

#include <vector>
#include <string>

#include "FindKeyStatement.hpp"
#include "InsertKeyStatement.hpp"

namespace anyreg
{
    class RegistryDatabase final
    {
    public:
        RegistryDatabase();

        void index(const std::vector<HKEY>& hives);
        void index(HKEY hive, const std::string& sub_path = {});

        void insert_key(const RegistryKeyEntry& key);
        void insert_value(const RegistryValueEntry& value);

        std::vector<RegistryKeyEntry> find_keys(const std::string& query);

    private:
        static sql::DatabaseConnection connect(const std::string& filename);
        
        sql::DatabaseConnection _db;
        InsertKeyStatement _insert_key_statement;
        sql::Statement _insert_value_statement;
        FindKeyStatement _find_key_statement;
    };
}
