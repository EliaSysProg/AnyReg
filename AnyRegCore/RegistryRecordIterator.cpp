#include "RegistryRecordIterator.hpp"

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
                .hive = reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(_statement->get_int64(1))),
                .path = _statement->get_text(2),
                .last_write_time = RegistryKeyTime(std::chrono::file_clock::duration(_statement->get_int64(3)))
            };
        }
        else
        {
            _current = {};
            _statement = nullptr;
        }

        return *this;
    }

    bool RegistryRecordIterator::operator!=(const RegistryRecordIterator& other) const
    {
        return _statement != other._statement;
    }
}
