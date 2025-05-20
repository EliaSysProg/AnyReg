#include "RegistryRecordRange.hpp"

#include "RegistryDatabase.hpp"

namespace anyreg
{
    RegistryRecordRange::RegistryRecordRange(const RegistryDatabase& db, std::vector<int64_t> ids)
        : _db(&db),
          _ids(std::move(ids))
    {
    }

    RegistryKeyView RegistryRecordRange::operator[](const size_t index) const
    {
        return _db->get_key(_ids[index]);
    }

    size_t RegistryRecordRange::size() const
    {
        return _ids.size();
    }
}
