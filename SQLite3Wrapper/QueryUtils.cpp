#include "QueryUtils.hpp"

namespace sql::query
{
    std::string fts_escape(const std::string_view user_query)
    {
        std::string result;
        result.reserve(user_query.size() + 2);
        result += '"';
        for (const char c : user_query)
        {
            switch (c)
            {
            case '\"':
                result += c;
                break;
            default:
                break;
            }

            result += c;
        }

        result += '"';

        return result;
    }

    std::string like_clause_escape(const std::string_view user_query, const char escape_char)
    {
        constexpr std::string_view escape_chars = "%_[]";
        std::string result;
        result.reserve(user_query.size());
        for (const char c : user_query)
        {
            if (c == escape_char || escape_chars.contains(c))
            {
                result += '|';
            }
            else if (c == '\'')
            {
                result += '\'';
            }

            result += c;
        }

        return result;
    }
}
