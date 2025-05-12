#pragma once

#include "RegistryEntry.hpp"

#include <Windows.h>

#include <functional>
#include <string_view>
#include <stop_token>

namespace anyreg
{
    void scan_registry(HKEY hive,
                       const std::function<void(const RegistryKeyView&)>& callback,
                       const std::stop_token& stop_token = {});

    void scan_registry(HKEY hive,
                       std::string_view path,
                       const std::function<void(const RegistryKeyView&)>& callback,
                       const std::stop_token& stop_token = {});
}
