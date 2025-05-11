#pragma once

#include <string>

namespace sql::query
{
    std::string fts_escape(const std::string& user_query);
    std::string like_clause_escape(const std::string& user_query, char escape_char);
}