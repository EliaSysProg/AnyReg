#pragma once

#include "RegistryEntry.hpp"

#include <vector>

namespace anyreg
{
    class RegistryDatabase;

    class RegistryRecordRange
    {
    public:
        explicit RegistryRecordRange(const RegistryDatabase& db, std::vector<int64_t> ids);

        [[nodiscard]] RegistryKeyView operator[](size_t index) const;
        [[nodiscard]] size_t size() const;

    private:
        const RegistryDatabase* _db;
        std::vector<int64_t> _ids;
    };
}
