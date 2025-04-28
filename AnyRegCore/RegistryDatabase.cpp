#include "RegistryDatabase.hpp"

#include <deque>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include "Registry.hpp"
#include "SQLite3Wrapper/ScopedTransaction.hpp"

namespace anyreg
{
    RegistryDatabase::RegistryDatabase(const int flags)
        : _db(connect(flags))
    {
        const std::filesystem::path path = LR"(C:\Windows\Temp\AnyReg.db)";
        if (std::filesystem::exists(path))
        {
            load(path);
        }
    }

    void RegistryDatabase::index(const std::vector<HKEY>& hives, const std::stop_token&)
    {
        for (const auto hive : hives)
        {
            index_hive(hive);
        }
    }

    void RegistryDatabase::save(const std::filesystem::path& filename) const
    {
        sql::database::backup(_db, sql::DatabaseConnection(filename.string()));
    }

    void RegistryDatabase::load(const std::filesystem::path& filename)
    {
        sql::database::backup(sql::DatabaseConnection(filename.string()), _db);
    }

    void RegistryDatabase::index_hive(const HKEY hive, const std::stop_token& stop_token)
    {
        index_sub_key(hive, "", stop_token);
    }

    void RegistryDatabase::index_sub_key(const HKEY hive, const std::string& path, const std::stop_token& stop_token)
    {
        std::deque<std::string> keys_to_process;
        // TODO: Add to DB
        keys_to_process.emplace_back(path);

        RegistryKeyEntry key_entry;
        RegistryValueEntry value_entry;
        std::string temp_name;

        size_t key_count = 0;

        while (!keys_to_process.empty())
        {
            if (stop_token.stop_requested())
            {
                OutputDebugStringW(L"Request to stop indexing hive at the middle of index_hive\r\n");
                return;
            }

            auto current_key = std::move(keys_to_process.front());
            keys_to_process.pop_front();

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
                keys_to_process.push_back(key_entry.get_full_path());
            }

            ++key_count;
        }
        OutputDebugStringW((L"Finished indexing hive: " + std::to_wstring(key_count) + L"\r\n").c_str());
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

    RegistryDatabase::FindKeyRange RegistryDatabase::find_keys(const std::string& query,
                                                               const std::stop_token& stop_token)
    {
        if (stop_token.stop_requested())
        {
            OutputDebugStringW(L"Request to stop find operation\r\n");
            return {};
        }
        _find_key_statement.reset_and_clear();
        _find_key_statement.bind(query);
        return FindKeyRange{&_find_key_statement};
    }

    sql::DatabaseConnection RegistryDatabase::connect(const int flags)
    {
        sql::DatabaseConnection db("AnyRegDb", flags);
        db.execute(R"(
CREATE TABLE IF NOT EXISTS RegistryKeys (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Name TEXT NOT NULL,
    Path TEXT NOT NULL,
    LastWriteTime INTEGER,
    UNIQUE(Name, Path)
);)");

        // Create FTS5 virtual table
        db.execute(R"(
CREATE VIRTUAL TABLE IF NOT EXISTS RegistryKeys_fts USING fts5(
    Name, 
    content='RegistryKeys',
    content_rowid='ID',
    tokenize='trigram');)");


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

        db.execute(R"(
CREATE TABLE IF NOT EXISTS RegistryValues (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Name TEXT NOT NULL,
    Path TEXT NOT NULL,
    Type INTEGER,
    UNIQUE(Name, Path)
);)");

        db.execute(R"(PRAGMA journal_mode=WAL)");

        return db;
    }
}
