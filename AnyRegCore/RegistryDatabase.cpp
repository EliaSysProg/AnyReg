#include "RegistryDatabase.hpp"

#include "Registry.hpp"

#include <filesystem>
#include <string>

namespace anyreg
{
    static constexpr int DEFAULT_FLAGS = SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_MEMORY | SQLITE_OPEN_URI;
    static constexpr auto DATABASE_NAME = "file:AnyRegDb?mode=memory&cache=shared";
    static constexpr auto DATABASE_FILE_NAME = "AnyReg.db";

    RegistryDatabase RegistryDatabase::create()
    {
        auto db = sql::DatabaseConnection(DATABASE_NAME, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | DEFAULT_FLAGS);
        if (std::filesystem::exists(DATABASE_FILE_NAME))
        {
            sql::database::backup(sql::DatabaseConnection(DATABASE_FILE_NAME), db);
        }

        db.execute(R"(
CREATE TABLE IF NOT EXISTS RegistryKeys (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    Name TEXT NOT NULL,
    ParentId INTEGER,
    LastWriteTime INTEGER,
    UNIQUE(Name, ParentId)
);)");

        // Create FTS5 virtual table
        // TODO: Sync with current RegistryKeys table
        db.execute(R"(CREATE VIRTUAL TABLE IF NOT EXISTS RegistryKeys_fts USING fts5(Name, content='RegistryKeys', tokenize='trigram');)");

        // Create triggers to keep FTS table in sync
        db.execute(R"(
CREATE TRIGGER IF NOT EXISTS RegistryKeys_ai AFTER INSERT ON RegistryKeys BEGIN
    INSERT INTO RegistryKeys_fts(rowid, Name) VALUES (new.rowid, new.Name);
END;)");

        db.execute(R"(
CREATE TRIGGER IF NOT EXISTS RegistryKeys_ad AFTER DELETE ON RegistryKeys BEGIN
    INSERT INTO RegistryKeys_fts(RegistryKeys_fts, rowid, Name) VALUES('delete', old.rowid, old.Name);
END;)");

        db.execute(R"(
CREATE TRIGGER IF NOT EXISTS RegistryKeys_au AFTER UPDATE ON RegistryKeys BEGIN
    INSERT INTO RegistryKeys_fts(RegistryKeys_fts, rowid, Name) VALUES('delete', old.rowid, old.Name);
    INSERT INTO RegistryKeys_fts(rowid, Name) VALUES (new.rowid, new.Name);
END;)");

        db.execute("PRAGMA journal_mode = WAL");

        db.execute("CREATE INDEX IF NOT EXISTS idx_registrykeys_name ON RegistryKeys(Name)");
        db.execute("CREATE INDEX IF NOT EXISTS idx_registrykeys_parentid ON RegistryKeys(ParentId)");
        db.execute("CREATE INDEX IF NOT EXISTS idx_registrykeys_lastwritetime ON RegistryKeys(LastWriteTime)");

        return RegistryDatabase{std::move(db)};
    }

    RegistryDatabase RegistryDatabase::open_read()
    {
        auto db = sql::DatabaseConnection(DATABASE_NAME, SQLITE_OPEN_READONLY | DEFAULT_FLAGS);
        return RegistryDatabase{std::move(db)};
    }

    RegistryDatabase RegistryDatabase::open_write()
    {
        auto db = sql::DatabaseConnection(DATABASE_NAME, DEFAULT_FLAGS | SQLITE_OPEN_READWRITE);
        if (!sqlite3_db_readonly(db.get(), DATABASE_NAME))
        {
            throw sql::DatabaseError("Database is not read-only");
        }

        return RegistryDatabase{std::move(db)};
    }

    void RegistryDatabase::save(const std::filesystem::path& filename) const
    {
        sql::database::backup(_db, sql::DatabaseConnection(filename.string()));
    }

    void RegistryDatabase::load(const std::filesystem::path& filename)
    {
        sql::database::backup(sql::DatabaseConnection(filename.string()), _db);
    }

    sql::ScopedTransaction RegistryDatabase::begin_scoped_transaction() const
    {
        return sql::ScopedTransaction(_db);
    }

    sql::ProgressHandler RegistryDatabase::create_progress_handler(std::function<bool()> callback) const
    {
        return {_db, std::move(callback)};
    }

    int64_t RegistryDatabase::insert_key(const RegistryKeyView& key)
    {
        _insert_key_statement.reset();

        _insert_key_statement.bind_text(1, key.name);
        if (key.parent_id > 0) // 0 - no parent (hive root key)
        {
            _insert_key_statement.bind_int64(2, key.parent_id);
        }
        const auto key_time_int = key.last_write_time.time_since_epoch().count();
        if (key_time_int > 0)
        {
            _insert_key_statement.bind_int64(3, key_time_int);
        }

        const auto row_available = _insert_key_statement.step();
        if (row_available)
        {
            throw sql::StatementError(std::format("Insert key: {} returned rows: {}", key.name, _insert_key_statement.get_sql()));
        }

        return last_insert_rowid();
    }

    RegistryKeyView RegistryDatabase::get_key(int64_t id) const
    {
        _get_key_statement.reset();
        _get_key_statement.bind_int64(1, id);
        if (!_get_key_statement.step())
        {
            throw sql::StatementError(std::format("Get key: {} Didn't return rows: {}", id, _get_key_statement.get_sql()));
        }

        const auto name = _get_key_statement.get_text(0);
        const auto parent_id = _get_key_statement.get_int64(1);
        const auto last_write_time = _get_key_statement.get_int64(2);


        return {
            .name = name,
            .parent_id = parent_id,
            .last_write_time = RegistryTime{std::chrono::file_clock::duration{last_write_time}}
        };
    }

    size_t RegistryDatabase::count_keys(const std::string_view query) const
    {
        const auto escaped_query = sql::query::fts_escape(query);
        sql::Statement count_statement(_db, "SELECT COUNT(*) FROM RegistryKeys_fts WHERE RegistryKeys_fts MATCH ?1");
        count_statement.bind_text(1, escaped_query);
        if (!count_statement.step())
        {
            throw sql::StatementError(std::format("Failed to execute count statement: {}", count_statement.get_sql()));
        }

        return static_cast<size_t>(count_statement.get_int64(0));
    }

    FindKeyStatement RegistryDatabase::find_keys(const SortColumn column, const SortOrder order) const
    {
        return FindKeyStatement{*this, column, order};
    }

    int64_t RegistryDatabase::last_insert_rowid() const
    {
        return _db.last_insert_rowid();
    }

    const sql::DatabaseConnection& RegistryDatabase::get() const
    {
        return _db;
    }

    RegistryDatabase::RegistryDatabase(sql::DatabaseConnection db)
        : _db(std::move(db)),
          _insert_key_statement(_db,
                                R"(
INSERT INTO RegistryKeys (Name, ParentId, LastWriteTime)
    VALUES (?, ?, ?)
ON CONFLICT(Name, ParentId) DO UPDATE
    SET LastWriteTime = excluded.LastWriteTime
    WHERE excluded.LastWriteTime > RegistryKeys.LastWriteTime;)"),
          _get_key_statement(_db, "SELECT Name, ParentId, LastWriteTime FROM RegistryKeys WHERE Id = ?;")
    {
    }
}
