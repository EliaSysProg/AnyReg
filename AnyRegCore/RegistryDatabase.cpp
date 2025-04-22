#include "RegistryDatabase.hpp"

#include <filesystem>

#include "Registry.hpp"

#include <string>
#include <system_error>

namespace anyreg
{
    RegistryDatabase::RegistryDatabase()
        : _db(connect(R"(C:\Windows\Temp\AnyReg.db)")),
          _insert_key_statement(_db),
          _insert_value_statement(_db,
                                  "INSERT INTO RegistryValues (Name, Path, Type) VALUES (?1, ?2, ?3) ON CONFLICT(Name, Path) DO UPDATE SET Type = excluded.Type WHERE excluded.Type != RegistryValues.Type;"),
          _find_key_statement(_db)
    {}

    void RegistryDatabase::index(const std::vector<HKEY>& hives)
    {
        for (const auto hive : hives)
        {
            index(hive);
        }
    }

    void RegistryDatabase::index(const HKEY hive, const std::string& sub_path)
    {
        std::vector<std::string> stack_keys;
        stack_keys.reserve(0x1000);
        // TODO: Add to DB
        stack_keys.emplace_back(std::begin(sub_path), std::end(sub_path));

        RegistryKeyEntry key_entry;
        RegistryValueEntry value_entry;
        std::string temp_name;

        _db.execute("BEGIN TRANSACTION;");

        while (!stack_keys.empty())
        {
            auto current_key = std::move(stack_keys.back());
            stack_keys.pop_back();

            RegistryKey key;

            try
            {
                key = RegistryKey(hive, current_key, KEY_READ);
            }
            catch (const std::system_error& e)
            {
                const auto error_code = e.code().value();
                if (error_code & (ERROR_ACCESS_DENIED | ERROR_PATH_NOT_FOUND))
                {
                    continue;
                }
            }

            // Enumerate values
            for (DWORD i = 0; key.get_value(i, value_entry.name, value_entry.type); ++i)
            {
                value_entry.path = current_key;
                insert_value(value_entry);
            }

            // Enumerate subkeys
            for (DWORD i = 0; key.get_sub_key(i, key_entry.name, key_entry.last_write_time); ++i)
            {
                key_entry.path = current_key;
                insert_key(key_entry);
                stack_keys.push_back(key_entry.get_full_path());
            }
        }

        _db.execute("COMMIT;");
    }

    void RegistryDatabase::insert_key(const RegistryKeyEntry& key)
    {
        _insert_key_statement.bind(key);
        _insert_key_statement.execute();
        _insert_key_statement.reset_and_clear();
    }

    void RegistryDatabase::insert_value(const RegistryValueEntry& value)
    {
        UNREFERENCED_PARAMETER(value);
    }

    std::vector<RegistryKeyEntry> RegistryDatabase::find_keys(const std::string& query)
    {
        std::vector<RegistryKeyEntry> keys;
        _find_key_statement.bind(query);
        for (_find_key_statement.step(); _find_key_statement.has_value(); _find_key_statement.step())
        {
            keys.push_back(_find_key_statement.get_value());
        }

        _find_key_statement.reset_and_clear();

        return keys;
    }

    sql::DatabaseConnection RegistryDatabase::connect(const std::string& filename)
    {
        sql::DatabaseConnection db(":memory:");
        if (std::filesystem::exists(filename))
        {
            const auto disk_db = sql::DatabaseConnection(filename);
            sql::database::backup(disk_db, db);
        }

        db.execute(R"(
CREATE TABLE IF NOT EXISTS RegistryKeys (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Name TEXT NOT NULL,
    Path TEXT NOT NULL,
    LastWriteTime INTEGER,
    UNIQUE(Name, Path)
);
)");

        // Create FTS5 virtual table
        db.execute(R"(
CREATE VIRTUAL TABLE IF NOT EXISTS RegistryKeys_fts USING fts5(
    Name, 
    content='RegistryKeys',
    content_rowid='ID',
    tokenize='trigram',
    prefix='1 2',  /* Enable prefix searches of length 1-2 */
    columnsize=0   /* Disable column size storage */
);)");


        // Create triggers to keep FTS table in sync
        db.execute(R"(
CREATE TRIGGER IF NOT EXISTS RegistryKeys_ai AFTER INSERT ON RegistryKeys BEGIN
    INSERT INTO RegistryKeys_fts(rowid, Name) 
    VALUES (new.ID, new.Name);
END;)");

        db.execute(R"(
CREATE TRIGGER IF NOT EXISTS RegistryKeys_ad AFTER DELETE ON RegistryKeys BEGIN
    INSERT INTO RegistryKeys_fts(RegistryKeys_fts, rowid, Name) 
    VALUES('delete', old.ID, old.Name);
END;)");

        db.execute(R"(
CREATE TRIGGER IF NOT EXISTS RegistryKeys_au AFTER UPDATE ON RegistryKeys BEGIN
    INSERT INTO RegistryKeys_fts(RegistryKeys_fts, rowid, Name) 
    VALUES('delete', old.ID, old.Name);
    INSERT INTO RegistryKeys_fts(rowid, Name) 
    VALUES (new.ID, new.Name);
END;)");

        // Create regular table for values
        db.execute(R"(
CREATE TABLE IF NOT EXISTS RegistryValues (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Name TEXT NOT NULL,
    Path TEXT NOT NULL,
    Type INTEGER,
    UNIQUE(Name, Path)
);
)");

        return db;
    }
}
