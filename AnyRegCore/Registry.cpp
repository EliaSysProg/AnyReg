#include "Registry.hpp"

#include "WinError.hpp"

namespace anyreg
{
    RegistryKey::RegistryKey(const HKEY root, const std::string& path, const REGSAM access)
    {
        const auto result = RegOpenKeyExA(root, path.data(), 0, access, &_key);
        if (result != ERROR_SUCCESS)
        {
            throw WinError(result);
        }
    }

    RegistryKey::~RegistryKey()
    {
        if (_key)
            RegCloseKey(_key);
    }

    RegistryKey::RegistryKey(RegistryKey&& other) noexcept
        : _key(std::exchange(other._key, nullptr))
    {}

    RegistryKey& RegistryKey::operator=(RegistryKey&& other) noexcept
    {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    DWORD RegistryKey::sub_key_count() const
    {
        DWORD count = 0;
        const auto status = RegQueryInfoKeyW(_key,
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             &count,
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             nullptr);
        if (status != ERROR_SUCCESS)
        {
            throw WinError(status);
        }

        return count;
    }

    bool RegistryKey::get_value(const DWORD index, std::string& name, RegistryValueType& type) const
    {
        DWORD temp_type;
        LSTATUS result = ERROR_SUCCESS;
        name.resize_and_overwrite(REGISTRY_MAX_KEY_VALUE_NAME,
                                  [&](char* const buffer, const size_t count)
                                  {
                                      DWORD size = static_cast<DWORD>(count);
                                      result = RegEnumValueA(_key,
                                                             index,
                                                             buffer,
                                                             &size,
                                                             nullptr,
                                                             &temp_type,
                                                             nullptr,
                                                             nullptr);
                                      return size;
                                  });

        if (result != ERROR_SUCCESS)
        {
            if (result == ERROR_NO_MORE_ITEMS)
            {
                return false;
            }

            throw WinError(result);
        }

        type = static_cast<RegistryValueType>(temp_type);

        return true;
    }

    bool RegistryKey::get_sub_key(const DWORD index, std::string& name, RegistryKeyTime& last_write_time) const
    {
        FILETIME file_time;
        LSTATUS result = ERROR_SUCCESS;
        name.resize_and_overwrite(REGISTRY_MAX_KEY_VALUE_NAME,
                                  [&](char* const buffer, const size_t count)
                                  {
                                      DWORD size = static_cast<DWORD>(count);
                                      result = RegEnumKeyExA(_key,
                                                             index,
                                                             buffer,
                                                             &size,
                                                             nullptr,
                                                             nullptr,
                                                             nullptr,
                                                             &file_time);
                                      return size;
                                  });

        if (result != ERROR_SUCCESS)
        {
            if (result == ERROR_NO_MORE_ITEMS)
            {
                return false;
            }

            throw WinError(result);
        }

        const std::chrono::file_clock::duration d{
            static_cast<int64_t>(file_time.dwHighDateTime) << 32 | file_time.dwLowDateTime
        };
        last_write_time = RegistryKeyTime{d};

        return true;
    }

    void swap(RegistryKey& first, RegistryKey& second) noexcept
    {
        using std::swap;
        swap(first._key, second._key);
    }
}
