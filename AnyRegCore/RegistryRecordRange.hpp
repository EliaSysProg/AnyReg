#pragma once

#include "RegistryRecordIterator.hpp"

namespace anyreg
{
    class RegistryRecordRange
    {
    public:
        explicit RegistryRecordRange(sql::Statement& statement);

        [[nodiscard]] RegistryRecordIterator begin() const;
        [[nodiscard]] RegistryRecordIterator end() const;

    private:
        sql::Statement* _statement = nullptr;
    };
}
