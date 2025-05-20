#include "RegistryIndexer.hpp"

#include "Registry.hpp"

#include <array>
#include <vector>

namespace anyreg
{
    static constexpr size_t REGISTRY_MAX_KEY_VALUE_NAME = 256;

    void scan_registry(RegistryDatabase& db, const HKEY hive, const std::stop_token& stop_token)
    {
        return scan_registry(db, hive, "", stop_token);
    }

    void scan_registry(RegistryDatabase& db, HKEY hive, std::string_view path, const std::stop_token& stop_token)
    {
        const auto transaction = db.begin_scoped_transaction();

        if (!is_predefined_hkey(hive))
        {
            throw std::invalid_argument("hive must be a predefined key");
        }

        struct KeyInfo final
        {
            RegistryKey key;
            int64_t id;
        };

        db.insert_key({.name = hive_name(hive), .parent_id = 0, .last_write_time = {}});

        std::vector<KeyInfo> keys_to_process;
        keys_to_process.reserve(512);
        keys_to_process.emplace_back(RegistryKey(hive, path, KEY_READ), db.last_insert_rowid());

        RegistryKeyView key_entry{};
        std::array<char, REGISTRY_MAX_KEY_VALUE_NAME> name_buffer{};
        std::span<char> name_span(name_buffer);

        while (!keys_to_process.empty())
        {
            if (stop_token.stop_requested())
            {
                return;
            }

            auto [parent_key, parent_id] = std::move(keys_to_process.back());
            keys_to_process.pop_back();

            for (DWORD i = 0; parent_key.get_sub_key(i, name_span, key_entry.last_write_time); ++i)
            {
                key_entry.name = std::string_view(name_span);
                key_entry.parent_id = parent_id;
                db.insert_key(key_entry);

                try
                {
                    auto sub_key = parent_key.open_sub_key(key_entry.name, KEY_READ);
                    const auto rowid = db.last_insert_rowid();
                    keys_to_process.emplace_back(std::move(sub_key), rowid);
                }
                catch (const std::system_error& e)
                {
                    const auto error_code = e.code().value();
                    if (!(error_code & (ERROR_ACCESS_DENIED | ERROR_PATH_NOT_FOUND))) // What can we do?
                    {
                        throw;
                    }
                }

                name_span = std::span(name_buffer);
            }
        }
    }
}
