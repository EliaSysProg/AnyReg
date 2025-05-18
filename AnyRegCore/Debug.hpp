#pragma once

#include <format>
#include <string>

#define TRACE(...) anyreg::debug::trace(std::format("[{}:{} | {}] {}\n", __FILE__, __LINE__, __FUNCTION__, std::format(__VA_ARGS__)))

namespace anyreg::debug
{
    void trace(const std::string& message);
}
