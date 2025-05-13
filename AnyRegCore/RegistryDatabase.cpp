#include "RegistryDatabase.hpp"

#include "Registry.hpp"

#include <filesystem>
#include <string>

namespace anyreg
{
    static constexpr int DEFAULT_FLAGS = SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_MEMORY | SQLITE_OPEN_URI;
    static constexpr auto DATABASE_NAME = "file:AnyRegDb?mode=memory&cache=shared";
    static constexpr auto DATABASE_FILE_NAME = R"(C:\Windows\Temp\AnyReg.db)";

    constexpr std::string column_name(const SortColumn sort_column)
    {
        switch (sort_column)
        {
        case SortColumn::NAME:
            return "Name";
        case SortColumn::PATH:
            return "Path";
        case SortColumn::LAST_WRITE_TIME:
            return "LastWriteTime";
        default:
            throw std::invalid_argument("Invalid SortColumn value");
        }
    }

    constexpr std::string order_name(const SortOrder sort_order)
    {
        switch (sort_order)
        {
        case SortOrder::ASCENDING:
            return "ASC";
        case SortOrder::DESCENDING:
            return "DESC";
        default:
            throw std::invalid_argument("Invalid SortOrder value");
        }
    }

    FindKeyStatement::FindKeyStatement(const sql::DatabaseConnection& db, const SortColumn column, const SortOrder order)
        : _statement(db, std::format(R"(
SELECT k.Name, k.Hive, k.Path, k.LastWriteTime
    FROM RegistryKeys k
INNER JOIN RegistryKeys_fts fts ON k.rowid = fts.rowid
    WHERE RegistryKeys_fts MATCH ?1
ORDER BY k.{} {} LIMIT ?3 OFFSET ?2;)", column_name(column), order_name(order)))
    {
    }

    void FindKeyStatement::bind(const std::string_view query)
    {
        _statement.bind_text(1, sql::query::fts_escape(query), true);
    }

    void FindKeyStatement::bind(const size_t offset, const size_t count)
    {
        _statement.bind_int64(2, static_cast<int64_t>(offset));
        _statement.bind_int64(3, static_cast<int64_t>(count));
    }

    void FindKeyStatement::reset()
    {
        _statement.reset();
    }

    RegistryRecordRange FindKeyStatement::find()
    {
        return RegistryRecordRange{_statement};   
    }

    RegistryDatabase RegistryDatabase::create()
    {
        auto db = sql::DatabaseConnection(DATABASE_NAME, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | DEFAULT_FLAGS);
        if (std::filesystem::exists(DATABASE_FILE_NAME))
        {
            sql::database::backup(sql::DatabaseConnection(DATABASE_FILE_NAME), db);
        }

        db.execute(R"(
CREATE TABLE IF NOT EXISTS RegistryKeys (
    Name TEXT NOT NULL,
    Hive INTEGER NOT NULL,
    Path TEXT NOT NULL,
    LastWriteTime INTEGER,
    UNIQUE(Hive, Name, Path)
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
        db.execute("CREATE INDEX IF NOT EXISTS idx_registrykeys_path ON RegistryKeys(Path)");
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

    void RegistryDatabase::insert_key(const RegistryKeyView& key)
    {
        _insert_key_statement.bind_text(1, key.name);
        _insert_key_statement.bind_int64(2, static_cast<int64_t>(key.hive));
        _insert_key_statement.bind_text(3, key.path);
        _insert_key_statement.bind_int64(4, key.last_write_time.time_since_epoch().count());

        if (_insert_key_statement.step())
        {
            throw sql::StatementError(std::format("Insert key: {} returned rows: {}", key.name, _insert_key_statement.get_sql()));
        }

        _insert_key_statement.reset();
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
        return FindKeyStatement{_db, column, order};
    }

    RegistryDatabase::RegistryDatabase(sql::DatabaseConnection db)
        : _db(std::move(db)),
          _insert_key_statement(_db,
                                R"(
INSERT INTO RegistryKeys (Name, Hive, Path, LastWriteTime)
    VALUES (?, ?, ?, ?)
ON CONFLICT(Hive, Name, Path) DO UPDATE
    SET LastWriteTime = excluded.LastWriteTime
    WHERE excluded.LastWriteTime > RegistryKeys.LastWriteTime;)")
    {
    }
}
