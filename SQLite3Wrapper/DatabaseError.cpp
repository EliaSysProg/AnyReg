#include "DatabaseError.hpp"

#include "SQLite3/sqlite3.h"

namespace sql
{
    DatabaseError::DatabaseError(const int error_code)
        : std::runtime_error(sqlite3_errstr(error_code))
    {
    }

    DatabaseError::DatabaseError(const std::string_view msg)
        : std::runtime_error({msg.data(), msg.size()})
    {
    }
}