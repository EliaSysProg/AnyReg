#include "RegistryIndexer.hpp"

#include "Registry.hpp"

namespace anyreg
{
    RegistryIndexer::RegistryIndexer(RegistryDatabase& db)
        : _db(&db)
    {
    }

    void RegistryIndexer::index(const uint64_t hive, const std::string_view path, const std::stop_token& stop_token) const
    {
        const auto transaction = _db->begin_scoped_transaction();

        scan_registry(reinterpret_cast<HKEY>(hive), path, [this](const RegistryKeyView& key)
        {
            _db->insert_key(key);
        }, stop_token);
    }
}
