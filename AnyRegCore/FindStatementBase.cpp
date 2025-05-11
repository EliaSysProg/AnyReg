#include "FindStatementBase.hpp"

namespace anyreg
{
    FindStatementBase::FindStatementBase(sql::Statement statement)
        : _statement(std::move(statement))
    {
    }

    void FindStatementBase::bind_range(size_t offset, size_t limit)
    {
        _statement.bind_int64(2, static_cast<int64_t>(offset));
        _statement.bind_int64(3, static_cast<int64_t>(limit));
    }

    void FindStatementBase::reset()
    {
        _statement.reset();
    }

    void FindStatementBase::clear_bindings()
    {
        _statement.clear_bindings();
    }

    RegistryRecordIterator FindStatementBase::begin()
    {
        return RegistryRecordIterator(&_statement);
    }

    RegistryRecordIterator FindStatementBase::end()
    {
        return {};
    }

    std::string FindStatementBase::get_sql() const
    {
        return _statement.get_sql();
    }

    void FindStatementBase::bind_query(const std::string& query, bool take_copy)
    {
        _statement.bind_text(1, query, take_copy);
    }
}
