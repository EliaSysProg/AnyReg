#include "Pch.hpp"
#include "Registry.hpp"

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

bool RegistryKey::get_value(const DWORD index, std::string& name, RegistryValueType& type) const
{
    auto temp_name = std::string(MAX_REGISTRY_KEY_VALUE_NAME, '\0');
    DWORD temp_type;
    DWORD name_size = static_cast<DWORD>(std::size(temp_name));
    const auto result = RegEnumValueA(_key, index, std::data(temp_name), &name_size, nullptr, &temp_type,
                                      nullptr, nullptr);
    if (result != ERROR_SUCCESS)
    {
        if (result == ERROR_NO_MORE_ITEMS)
        {
            return false;
        }

        throw std::system_error(result, std::system_category());
    }

    temp_name.resize(name_size);

    name = std::move(temp_name);
    type = static_cast<RegistryValueType>(temp_type);
    return true;
}

bool RegistryKey::get_sub_key(const DWORD index, std::string& name, RegistryKeyTime& last_write_time) const
{
    auto temp_name = std::string(MAX_REGISTRY_KEY_VALUE_NAME, '\0');
    DWORD name_size = static_cast<DWORD>(std::size(temp_name));
    FILETIME filetime;
    const auto result = RegEnumKeyExA(_key, index, std::data(temp_name), &name_size, nullptr, nullptr, nullptr, &filetime);
    if (result != ERROR_SUCCESS)
    {
        if (result == ERROR_NO_MORE_ITEMS)
        {
            return false;
        }

        throw std::system_error(result, std::system_category());
    }
    
    temp_name.resize(name_size);
    const std::chrono::file_clock::duration d{static_cast<int64_t>(filetime.dwHighDateTime) << 32 | filetime.dwLowDateTime};

    name = std::move(temp_name);
    last_write_time = RegistryKeyTime{d};
    return true;
}

void swap(RegistryKey& first, RegistryKey& second) noexcept
{
    using std::swap;
    swap(first._key, second._key);
}
