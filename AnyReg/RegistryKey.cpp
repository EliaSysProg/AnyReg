#include "Pch.hpp"
#include "RegistryKey.hpp"

RegistryKey::RegistryKey(const HKEY root, const std::string_view path, const REGSAM access = KEY_READ)
{
    const auto result = RegOpenKeyExA(root, path.data(), 0, access, &_key);
    if (result != ERROR_SUCCESS)
    {
        throw std::system_error(result, std::system_category());
    }
}

RegistryKey::~RegistryKey()
{
    if (_key)
        RegCloseKey(_key);
}

RegistryKey::RegistryKey(RegistryKey&& other) noexcept
    : _key(std::exchange(other._key, nullptr))
{
}

RegistryKey& RegistryKey::operator=(RegistryKey&& other) noexcept
{
    using std::swap;
    swap(*this, other);
    return *this;
}

HKEY RegistryKey::get()
{
    return _key;
}

bool RegistryKey::get_value(const DWORD index, std::span<char> name) const
{
    DWORD name_size = static_cast<DWORD>(name.size());
    const auto result = RegEnumValueA(_key, index, name.data(), &name_size, nullptr, nullptr, nullptr, nullptr);
    if (result != ERROR_SUCCESS)
    {
        if (result == ERROR_NO_MORE_ITEMS)
        {
            return false;
        }

        throw std::system_error(result, std::system_category());
    }

    return true;
}

bool RegistryKey::get_sub_key(const DWORD index, std::span<char> name) const
{
    DWORD name_size = static_cast<DWORD>(name.size());
    const auto result = RegEnumKeyExA(_key, index, name.data(), &name_size, nullptr, nullptr, nullptr, nullptr);
    if (result != ERROR_SUCCESS)
    {
        if (result == ERROR_NO_MORE_ITEMS)
        {
            return false;
        }

        throw std::system_error(result, std::system_category());
    }

    return true;
}

void swap(RegistryKey& first, RegistryKey& second) noexcept
{
    using std::swap;
    swap(first._key, second._key);
}
