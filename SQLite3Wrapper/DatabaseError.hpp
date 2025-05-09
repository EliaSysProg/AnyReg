#pragma once

#include <stdexcept>
#include <stacktrace>
#include <string_view>

namespace sql
{
    class DatabaseError : public std::runtime_error
    {
    public:
        explicit DatabaseError(int error_code);
        explicit DatabaseError(std::string_view msg);
        ~DatabaseError() override = default;

        [[nodiscard]] const std::stacktrace& stacktrace() const noexcept;

    private:
        std::stacktrace _stacktrace;
    };
}
