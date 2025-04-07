#pragma once

#include <stdexcept>
#include <string_view>

namespace sql
{
class DatabaseError : public std::runtime_error
{
public:
    explicit DatabaseError(int error_code);
    explicit DatabaseError(std::string_view msg);
    ~DatabaseError() override = default;
};
}
