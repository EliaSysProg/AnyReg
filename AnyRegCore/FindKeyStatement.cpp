#include "FindKeyStatement.hpp"

namespace anyreg
{
    FindKeyStatement::FindKeyStatement(const sql::DatabaseConnection& db)
        : _statement(db, "SELECT Name, Path, LastWriteTime FROM RegistryKeys WHERE Name LIKE '%' || ?1 || '%' LIMIT 100;")
    {}

    void FindKeyStatement::bind(const std::string& query)
    {
        _statement.bind_text(1, query);
    }

    bool FindKeyStatement::has_value() const
    {
        return _has_value;
    }

    RegistryKeyEntry FindKeyStatement::get_value()
    {
        RegistryKeyEntry entry{
            .name = _statement.get_text(0),
            .path = _statement.get_text(1),
            .last_write_time = RegistryKeyTime(std::chrono::file_clock::duration(_statement.get_int64(2)))
        };
        return entry;
    }

    void FindKeyStatement::step()
    {
        switch (const auto error_code = _statement.step())
        {
        case SQLITE_ROW:
            _has_value = true;
            break;
        case SQLITE_DONE:
            _has_value = false;
            break;
        default:
            throw sql::StatementError(error_code);
        }
    }

    void FindKeyStatement::reset_and_clear()
    {
        _statement.reset();
        _statement.clear_bindings();
    }
}
