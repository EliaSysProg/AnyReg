#include "RegistryEntry.hpp"

std::string RegistryKeyEntry::get_full_path() const
{
    std::string sub_key_path = path;
    if (!sub_key_path.empty() && !sub_key_path.ends_with("\\"))
    {
        sub_key_path.push_back('\\');
    }
    sub_key_path.append(name);
    return sub_key_path;
}
