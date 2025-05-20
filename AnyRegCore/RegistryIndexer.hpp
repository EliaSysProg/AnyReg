#pragma once

#include "Registry.hpp"
#include "RegistryDatabase.hpp"

#include <stop_token>
#include <string_view>

namespace anyreg
{
    void scan_registry(RegistryDatabase& db, HKEY hive, const std::stop_token& stop_token = {});
    void scan_registry(RegistryDatabase& db, HKEY hive, std::string_view path, const std::stop_token& stop_token = {});
}
