#include "DatabaseBackup.hpp"

namespace sql
{
    void DatabaseBackup::DatabaseBackupDeleter::operator()(sqlite3_backup* backup) const noexcept
    {
        sqlite3_backup_finish(backup);
    }

    DatabaseBackup::DatabaseBackup(const DatabaseConnection& source,
                                   const DatabaseConnection& destination,
                                   const std::string& source_name,
                                   const std::string& destination_name)
        : _sqlite3_backup(sqlite3_backup_init(destination.get(),
                                              destination_name.c_str(),
                                              source.get(),
                                              source_name.c_str()))
    {
        if (!_sqlite3_backup)
        {
            throw BackupError(destination.errmsg());
        }
    }

    void DatabaseBackup::step()
    {
        sqlite3_backup_step(_sqlite3_backup.get(), -1);
    }
}
