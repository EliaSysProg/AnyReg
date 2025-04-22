#pragma once

#include "DatabaseConnection.hpp"

namespace sql::database
{
    void backup(const DatabaseConnection& source,
                const DatabaseConnection& destination,
                const std::string& source_name = "main",
                const std::string& destination_name = "main");
}
