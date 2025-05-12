#include "RegistryRecordRange.hpp"

namespace anyreg
{
    RegistryRecordRange::RegistryRecordRange(sql::Statement* statement)
        : _statement(statement)
    {
    }

    RegistryRecordIterator RegistryRecordRange::begin() const
    {
        return RegistryRecordIterator{_statement};
    }

    RegistryRecordIterator RegistryRecordRange::end() const
    {
        return {};
    }
}
