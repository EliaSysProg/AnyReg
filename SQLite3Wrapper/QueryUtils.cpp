#include "QueryUtils.hpp"

namespace sql::query
{
    std::string fts_escape(const std::string& user_query)
    {
        std::string result;
        result.reserve(user_query.size() * 2);
        result += '"';
        for (const char it : user_query)
        {
            switch (it)
            {
            case '\'':
                result += '\'';
                break;
            case '"':
                result += '\"';
                break;
            default:
                break;
            }

            result += it;
        }

        result += '"';

        return result;
    }
}
