#include "RegistryEntry.hpp"

std::wstring RegistryKeyEntry::get_full_path() const
{
    std::wstring sub_key_path = path;
    if (!sub_key_path.empty() && !sub_key_path.ends_with(L"\\"))
    {
        sub_key_path.push_back(L'\\');
    }
    sub_key_path.append(name);
    return sub_key_path;
}
