#include "Registry.hpp"

#include "WinError.hpp"

#include <deque>
#include <array>
#include <span>

static constexpr size_t REGISTRY_MAX_KEY_VALUE_NAME = 256;

namespace anyreg
{
    constexpr static bool is_predefined_hkey(const HKEY hkey)
    {
        for (const auto hive : {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_CLASSES_ROOT, HKEY_USERS, HKEY_CURRENT_CONFIG})
        {
            if (hkey == hive)
            {
                return true;
            }
        }

        return false;
    }

    class RegistryKey
    {
    public:
        RegistryKey() = default;
        explicit RegistryKey(HKEY root, const std::string& path, REGSAM access = KEY_READ);
        ~RegistryKey();

        RegistryKey(RegistryKey&& other) noexcept;
        RegistryKey& operator=(RegistryKey&& other) noexcept;

        RegistryKey(const RegistryKey& other) = delete;
        RegistryKey& operator=(const RegistryKey& other) = delete;

        [[nodiscard]] DWORD sub_key_count() const;

        [[nodiscard]] bool get_sub_key(DWORD index, std::span<char>& name, RegistryTime& last_write_time) const;

        friend void swap(RegistryKey& first, RegistryKey& second) noexcept;

    private:
        HKEY _key{};
    };

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

    void swap(RegistryKey& first, RegistryKey& second) noexcept
    {
        using std::swap;
        swap(first._key, second._key);
    }

    void scan_registry(const HKEY hive, const std::function<void(const RegistryKeyView&)>& callback, const std::stop_token& stop_token)
    {
        return scan_registry(hive, "", callback, stop_token);
    }

    void scan_registry(const HKEY hive, const std::string_view path, const std::function<void(const RegistryKeyView&)>& callback, const std::stop_token& stop_token)
    {
        if (!is_predefined_hkey(hive))
        {
            throw std::invalid_argument("hive must be a predefined key");
        }

        std::deque<std::string> keys_to_process;
        keys_to_process.emplace_back(path);

        RegistryKeyView key_entry{};
        std::array<char, REGISTRY_MAX_KEY_VALUE_NAME> name_buffer{};
        std::span<char> name_span(name_buffer);

        while (!keys_to_process.empty())
        {
            if (stop_token.stop_requested())
            {
                return;
            }

            auto current_key = std::move(keys_to_process.front());
            keys_to_process.pop_front();

            RegistryKey key;

            try
            {
                key = RegistryKey(hive, current_key, KEY_READ);
            }
            catch (const std::system_error& e)
            {
                const auto error_code = e.code().value();
                if (error_code & (ERROR_ACCESS_DENIED | ERROR_PATH_NOT_FOUND)) // What can we do?
                {
                    continue;
                }
            }

            for (DWORD i = 0; key.get_sub_key(i, name_span, key_entry.last_write_time); ++i)
            {
                key_entry.name = std::string_view(name_span);
                key_entry.path = current_key;
                key_entry.hive = reinterpret_cast<uint64_t>(hive);
                callback(key_entry);
                keys_to_process.push_back(key_entry.hive_relative_path());
                name_span = std::span(name_buffer);
            }
        }
    }
}
