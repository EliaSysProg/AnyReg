#pragma once

#include <Windows.h>

#include <string>
#include <system_error>

class WinError final : public std::system_error
{
public:
    WinError();
    explicit WinError(DWORD error_code);

    static std::string get_win32_error_message(DWORD error_code);
};
