#pragma once

#include "RegistryDatabase.hpp"

#include <stop_token>
#include <string_view>

namespace anyreg
{
    class RegistryIndexer final
    {
    public:
        explicit RegistryIndexer(RegistryDatabase& db);

        void index(uint64_t hive, std::string_view path = "", const std::stop_token& stop_token = {}) const;

    private:
        RegistryDatabase* _db;
    };
}