#pragma once

#include <memory>

#include "DatabaseConnection.hpp"

namespace sql
{
    class DatabaseBackup
    {
        struct DatabaseBackupDeleter final
        {
            void operator()(sqlite3_backup* backup) const noexcept;
        };

    public:
        DatabaseBackup(const DatabaseConnection& source,
                       const DatabaseConnection& destination,
                       const std::string& source_name = "main",
                       const std::string& destination_name = "main");


        void step();

    private:
        std::unique_ptr<sqlite3_backup, DatabaseBackupDeleter> _sqlite3_backup;
    };

    class BackupError final : public DatabaseError
    {
    public:
        using DatabaseError::DatabaseError;
    };
}
