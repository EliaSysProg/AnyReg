#pragma once

#include "RegistryEntry.hpp"

#include <iterator>

namespace sql
{
    class Statement;
}

namespace anyreg
{
    class RegistryRecordIterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = RegistryKeyView;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        RegistryRecordIterator() = default;
        explicit RegistryRecordIterator(sql::Statement* statement);

        const RegistryKeyView& operator*() const;
        const RegistryKeyView* operator->() const;
        RegistryRecordIterator& operator++();
        RegistryRecordIterator operator++(int);
        bool operator!=(const RegistryRecordIterator& other) const;
        bool operator==(const RegistryRecordIterator& other) const;

    private:
        sql::Statement* _statement = nullptr;
        RegistryKeyView _current{};
    };
}
