#include "RegistryDatabase.hpp"

#include <deque>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>
#include <array>

#include "Registry.hpp"
#include "SQLite3Wrapper/ScopedTransaction.hpp"

namespace anyreg
{
    constexpr static bool is_predefined_hkey(const HKEY hkey)
    {
        static const std::array KNOWN_HIVES = {
            HKEY_LOCAL_MACHINE,
            HKEY_CURRENT_USER,
            HKEY_CLASSES_ROOT,
            HKEY_USERS,
            HKEY_CURRENT_CONFIG
        };

        return std::ranges::contains(KNOWN_HIVES, hkey);
    }

    RegistryDatabase::RegistryDatabase(const int flags)
        : _db(connect(flags))
    {
        const std::filesystem::path path = LR"(C:\Windows\Temp\AnyReg.db)";
        if (std::filesystem::exists(path))
        {
            load(path);
        }
    }

    void RegistryDatabase::index(const std::span<const HKEY> hives, const std::stop_token&)
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
        if (!is_predefined_hkey(hive))
        {
            throw std::invalid_argument("hive must be a predefined key");
        }

        std::deque<std::string> keys_to_process;
        keys_to_process.emplace_back("");

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
                key_entry.hive = hive;
                insert_key(key_entry);
                keys_to_process.push_back(std::filesystem::path(key_entry.path).append(key_entry.name).string());
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

    RegistryDatabase::FindKeyRange::FindKeyRange(std::shared_ptr<FindKeyStatement> statement): _statement(std::move(statement))
    {
    }

    FindKeyStatement::iterator RegistryDatabase::FindKeyRange::begin() const
    {
        return _statement ? _statement->begin() : FindKeyStatement::iterator{};
    }

    FindKeyStatement::iterator RegistryDatabase::FindKeyRange::end() const
    {
        return _statement ? _statement->end() : FindKeyStatement::iterator{};
    }

    RegistryDatabase::FindKeyRange RegistryDatabase::find_keys(const std::string& query,
                                                               const std::stop_token& stop_token)
    {
        return find_keys(query, FindKeyStatement::SortColumn::NAME, FindKeyStatement::SortOrder::ASCENDING, stop_token);
    }

    RegistryDatabase::FindKeyRange RegistryDatabase::find_keys(const std::string& query,
                                                               FindKeyStatement::SortColumn sort_column,
                                                               FindKeyStatement::SortOrder order,
                                                               const std::stop_token& stop_token)
    {
        if (stop_token.stop_requested())
        {
            OutputDebugStringW(L"Request to stop find operation\r\n");
            return {};
        }

        return FindKeyRange{std::make_shared<FindKeyStatement>(_db, query, sort_column, order)};
    }

    sql::DatabaseConnection RegistryDatabase::connect(const int flags)
    {
        sql::DatabaseConnection db("AnyRegDb", flags);
        db.execute(R"(
CREATE TABLE IF NOT EXISTS RegistryKeys (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Name TEXT NOT NULL,
    Hive INTEGER NOT NULL,
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
    Hive INTEGER NOT NULL,
    Path TEXT NOT NULL,
    Type INTEGER,
    UNIQUE(Name, Path)
);)");

        db.execute("PRAGMA journal_mode=WAL");
        db.execute("PRAGMA temp_store = MEMORY");
        db.execute("PRAGMA cache_size = 1000000;");

        db.execute("CREATE INDEX idx_registrykeys_name ON RegistryKeys(Name);");
        db.execute("CREATE INDEX idx_registrykeys_path ON RegistryKeys(Path);");
        db.execute("CREATE INDEX idx_registrykeys_lastwritetime ON RegistryKeys(LastWriteTime);");

        return db;
    }
}
