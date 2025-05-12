#pragma once

#include <string>
#include <string_view>

namespace sql::query
{
    std::string fts_escape(std::string_view user_query);
    std::string like_clause_escape(std::string_view user_query, char escape_char);
}