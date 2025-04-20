#include "InsertKeyStatement.hpp"

namespace anyreg
{
    InsertKeyStatement::InsertKeyStatement(const sql::DatabaseConnection& db)
        : _statement(db,
                     "INSERT INTO RegistryKeys (Name, Path, LastWriteTime) VALUES (?1, ?2, ?3) ON CONFLICT(Name, Path) DO UPDATE SET LastWriteTime = excluded.LastWriteTime WHERE excluded.LastWriteTime > RegistryKeys.LastWriteTime;")
    {}

    void InsertKeyStatement::bind(const RegistryKeyEntry& entry)
    {
        _statement.bind_text(1, entry.name);
        _statement.bind_text(2, entry.path);
        _statement.bind_int64(3, entry.last_write_time.time_since_epoch().count());
    }

    void InsertKeyStatement::execute()
    {
        const auto error_code = _statement.step();
        if (error_code != SQLITE_DONE)
        {
            throw sql::StatementError(error_code);
        }
    }

    void InsertKeyStatement::reset_and_clear()
    {
        _statement.reset();
        _statement.clear_bindings();
    }
}
