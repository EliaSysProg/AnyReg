#include "DatabaseUtils.hpp"

#include "DatabaseBackup.hpp"

namespace sql::database
{
    void backup(const DatabaseConnection& source,
                const DatabaseConnection& destination,
                const std::string& source_name,
                const std::string& destination_name)
    {
        DatabaseBackup(source, destination, source_name, destination_name).step();
    }
}
