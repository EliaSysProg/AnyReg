#include "Registry.hpp"

#include "WinError.hpp"

namespace anyreg
{
    std::expected<RegistryKey, DWORD> RegistryKey::try_open(HKEY root, std::string_view path, REGSAM access)
    {
        HKEY key{};
        const auto result = RegOpenKeyExA(root, path.data(), 0, access, &key);
        if (result != ERROR_SUCCESS)
        {
            return std::unexpected(result);
        }

        return RegistryKey{key};
    }

    RegistryKey::RegistryKey(const HKEY root, const std::string_view path, const REGSAM access)
    {
        const auto result = RegOpenKeyExA(root, path.data(), 0, access, &_key);
        if (result != ERROR_SUCCESS)
        {
            throw WinError(result);
        }
    }

    RegistryKey::~RegistryKey()
    {
        if (_key && !is_predefined_hkey(_key))
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

    bool RegistryKey::get_sub_key(const DWORD index, std::span<char>& name, RegistryTime& last_write_time) const
    {
        FILETIME file_time;
        DWORD size = static_cast<DWORD>(name.size());

        const LSTATUS result = RegEnumKeyExA(_key,
                                             index,
                                             name.data(),
                                             &size,
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             &file_time);

        if (result != ERROR_SUCCESS)
        {
            if (result == ERROR_NO_MORE_ITEMS)
            {
                return false;
            }

            throw WinError(result);
        }

        name = name.first(size);

        const std::chrono::file_clock::duration d{
            static_cast<int64_t>(file_time.dwHighDateTime) << 32 | file_time.dwLowDateTime
        };
        last_write_time = RegistryTime{d};

        return true;
    }

    RegistryKey RegistryKey::open_sub_key(const std::string_view path, const REGSAM access) const
    {
        return RegistryKey(_key, path, access);
    }

    std::expected<RegistryKey, DWORD> RegistryKey::try_open_sub_key(std::string_view path, REGSAM access) const
    {
        return try_open(_key, path, access);
    }

    RegistryKey::RegistryKey(HKEY key)
        : _key(key)
    {
    }

    void swap(RegistryKey& first, RegistryKey& second) noexcept
    {
        using std::swap;
        swap(first._key, second._key);
    }
}
