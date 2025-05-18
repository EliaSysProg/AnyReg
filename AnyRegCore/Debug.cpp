#include "Debug.hpp"

#include <Windows.h>

namespace anyreg::debug
{
    void trace(const std::string& message)
    {
        OutputDebugStringA(message.data());
    }
}
