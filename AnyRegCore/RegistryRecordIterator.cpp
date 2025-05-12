#include "RegistryRecordIterator.hpp"

#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

#include <Windows.h>

namespace anyreg
{
    RegistryRecordIterator::RegistryRecordIterator(sql::Statement* statement)
        : _statement(statement)
    {
        ++*this; // Get the first element
    }

    const RegistryKeyView& RegistryRecordIterator::operator*() const
    {
        return _current;
    }

    const RegistryKeyView* RegistryRecordIterator::operator->() const
    {
        return &_current;
    }

    RegistryRecordIterator& RegistryRecordIterator::operator++()
    {
        if (!_statement)
        {
            return *this;
        }

        if (_statement->step())
        {
            _current = RegistryKeyView{
                .name = _statement->get_text(0),
                .hive = static_cast<uint64_t>(_statement->get_int64(1)),
                .path = _statement->get_text(2),
                .last_write_time = RegistryTime(std::chrono::file_clock::duration(_statement->get_int64(3)))
            };
        }
        else
        {
            _current = {};
            _statement = nullptr;
        }

        return *this;
    }

    RegistryRecordIterator RegistryRecordIterator::operator++(int)
    {
        const auto copy = *this;
        ++*this;
        return copy;
    }

    bool RegistryRecordIterator::operator!=(const RegistryRecordIterator& other) const
    {
        return !(*this == other);
    }

    bool RegistryRecordIterator::operator==(const RegistryRecordIterator& other) const
    {
        return _statement == other._statement;
    }
}
