#pragma once

#include <string>

namespace sql::query
{
    std::string fts_escape(const std::string& user_query);
}