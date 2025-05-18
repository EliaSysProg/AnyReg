#include "AnyRegCore.hpp"

#include "Debug.hpp"
#include "Registry.hpp"

#include <Windows.h>

#include <array>
#include <filesystem>
#include <ranges>
#include <vector>

namespace anyreg
{
    static constexpr size_t REGISTRY_MAX_KEY_VALUE_NAME = 256;

    constexpr static std::string_view hive_name(const HKEY hive)
    {
        if (hive == HKEY_CLASSES_ROOT) return "HKEY_CLASSES_ROOT";
        if (hive == HKEY_CURRENT_USER) return "HKEY_CURRENT_USER";
        if (hive == HKEY_LOCAL_MACHINE) return "HKEY_LOCAL_MACHINE";
        if (hive == HKEY_USERS) return "HKEY_USERS";
        if (hive == HKEY_CURRENT_CONFIG) return "HKEY_CURRENT_CONFIG";
        return "";
    }

    std::string RegistryKeyView::full_path() const
    {
        return (std::filesystem::path(hive_name(reinterpret_cast<HKEY>(hive))) / hive_relative_path()).string();
    }

    std::string RegistryKeyView::hive_relative_path() const
    {
        return (std::filesystem::path(path) / name).string();
    }

    void index_hive(RegistryDatabase& db, const HKEY hive, const std::stop_token& stop_token)
    {
        index_hive(db, hive, "", stop_token);
    }

    void index_hive(RegistryDatabase& db, const HKEY hive, const std::string_view path, const std::stop_token& stop_token)
    {
        if (!is_predefined_hkey(hive))
        {
            throw std::invalid_argument("hive must be a predefined key");
        }

        TRACE("Indexing hive {}", hive_name(hive));

        const size_t db_size = db.size();
        RegistryId next_available_id = 1 + (db.empty() ? 0 : *std::ranges::max_element(db | std::views::keys));

        struct KeyInfo final
        {
            RegistryId id;
            RegistryKey key;
        };

        std::vector<KeyInfo> keys_to_process;
        keys_to_process.reserve(512);
        keys_to_process.emplace_back(next_available_id, RegistryKey(hive, std::string(path), KEY_READ));

        db.emplace(std::piecewise_construct,
                   std::forward_as_tuple(next_available_id),
                   std::forward_as_tuple(std::string(hive_name(hive)), RegistryTime{}, 0));

        ++next_available_id;

        std::array<char, REGISTRY_MAX_KEY_VALUE_NAME> name_buffer{};
        std::span<char> name_span(name_buffer);
        RegistryTime last_write_time{};

        while (!keys_to_process.empty())
        {
            if (stop_token.stop_requested())
            {
                return;
            }

            auto [parent_id, parent_reg_key] = std::move(keys_to_process.back());
            keys_to_process.pop_back();

            for (DWORD i = 0; parent_reg_key.get_sub_key(i, name_span, last_write_time); ++i, name_span = std::span(name_buffer))
            {
                // Insert to DB
                const auto name = std::string_view(name_span);
                auto [it, success] = db.emplace(std::piecewise_construct,
                                                std::forward_as_tuple(next_available_id),
                                                std::forward_as_tuple(std::string(name), last_write_time, parent_id));

                if (!success)
                {
                    TRACE("Id already exists: {} -> {}", next_available_id, name);
                    throw std::runtime_error("Failed to insert key into database");
                }

                // Add child to processing
                try
                {
                    keys_to_process.emplace_back(next_available_id, parent_reg_key.sub_key(name));
                }
                catch (const std::system_error& e)
                {
                    const auto error_code = e.code().value();
                    if (!(error_code & (ERROR_ACCESS_DENIED | ERROR_PATH_NOT_FOUND))) // What can we do?
                    {
                        throw;
                    }
                }

                ++next_available_id;
            }
        }

        TRACE("Finished indexing hive {} with {} keys", hive_name(hive), db.size() - db_size);
    }

    std::string key_full_path(const RegistryDatabase& db, RegistryId id)
    {
        std::string full_path;
        while (id != 0)
        {
            const auto& entry = db.at(id);
            full_path = std::format("{}\\{}", entry.name, full_path);
            id = entry.parent_id;
        }

        return full_path;
    }
}
