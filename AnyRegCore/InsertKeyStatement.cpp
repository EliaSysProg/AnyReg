#include "InsertKeyStatement.hpp"

namespace anyreg
{
    InsertKeyStatement::InsertKeyStatement(const sql::DatabaseConnection& db)
        : _statement(db,
                     "INSERT INTO RegistryKeys (Name, Hive, Path, LastWriteTime) VALUES (?, ?, ?, ?) ON CONFLICT(Name, Path) DO UPDATE SET LastWriteTime = excluded.LastWriteTime WHERE excluded.LastWriteTime > RegistryKeys.LastWriteTime;")
    {
    }

    void InsertKeyStatement::bind(const RegistryKeyEntry& entry)
    {
        _statement.bind_text(1, entry.name);
        _statement.bind_int64(2, static_cast<INT64>(reinterpret_cast<ULONG_PTR>(entry.hive)));
        _statement.bind_text(3, entry.path);
        _statement.bind_int64(4, entry.last_write_time.time_since_epoch().count());
    }

    void InsertKeyStatement::execute()
    {
        if (_statement.step())
        {
            throw sql::StatementError("InsertKeyStatement step() should not return rows");
        }
    }

    void InsertKeyStatement::reset_and_clear()
    {
        _statement.reset();
        _statement.clear_bindings();
    }
}
