#include "WinError.hpp"

WinError::WinError()
    : WinError(GetLastError())
{
}

WinError::WinError(const DWORD error_code)
    : std::system_error(static_cast<int>(error_code), std::system_category(), get_win32_error_message(error_code))
{
}

std::string WinError::get_win32_error_message(const DWORD error_code)
{
    LPSTR message_buffer = nullptr;

    const DWORD size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error_code,
        0, // Default language
        reinterpret_cast<LPSTR>(&message_buffer),
        0,
        nullptr
    );

    std::string message;
    if (size > 0 && message_buffer)
    {
        // TODO: This can throw
        message.assign(message_buffer, size);
        // Remove trailing newlines and carriage returns
        while (!message.empty() && (message.back() == '\r' || message.back() == '\n'))
            message.pop_back();
    }
    else
    {
        message = "Unknown error code: " + std::to_string(error_code);
    }

    // TODO: RAII
    if (message_buffer)
        LocalFree(message_buffer);

    return message;
}
