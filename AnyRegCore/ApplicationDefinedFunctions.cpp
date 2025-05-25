#include "ApplicationDefinedFunctions.hpp"

#include <algorithm>
#include <ranges>
#include <string>
#include <vector>

namespace anyreg
{
    void full_path_udf(sqlite3_context* ctx, int argc, sqlite3_value** argv)
    {
        try
        {
            if (argc != 1 || sqlite3_value_type(argv[0]) != SQLITE_INTEGER)
            {
                sqlite3_result_null(ctx);
                return;
            }

            sqlite3* const db = sqlite3_context_db_handle(ctx);
            sqlite3_int64 id = sqlite3_value_int64(argv[0]);

            sqlite3_stmt* stmt = nullptr;
            std::vector<std::string> names;

            constexpr char sql[] = "SELECT Name, ParentId FROM RegistryKeys WHERE ID = ?";
            if (sqlite3_prepare_v2(db, std::data(sql), std::size(sql), &stmt, nullptr) != SQLITE_OK)
            {
                sqlite3_result_error(ctx, "Failed to prepare statement", -1);
                return;
            }

            while (id != 0)
            {
                auto error_code = sqlite3_bind_int64(stmt, 1, id);
                if (error_code != SQLITE_OK)
                {
                    sqlite3_result_error(ctx, "Statement bind failed", -1);
                    break;
                }

                error_code = sqlite3_step(stmt);
                if (error_code == SQLITE_ROW)
                {
                    const unsigned char* name = sqlite3_column_text(stmt, 0);
                    id = sqlite3_column_type(stmt, 1) == SQLITE_NULL ? 0 : sqlite3_column_int64(stmt, 1);
                    names.emplace_back(reinterpret_cast<const char*>(name));
                }
                else if (error_code == SQLITE_DONE)
                {
                    break; // ID not found
                }
                else
                {
                    sqlite3_result_error(ctx, "Statement step failed", -1);
                    break;
                }

                error_code = sqlite3_reset(stmt);
                if (error_code != SQLITE_OK)
                {
                    sqlite3_result_error(ctx, "Statement reset failed", -1);
                    break;
                }
            }

            sqlite3_finalize(stmt);
            stmt = nullptr;

            std::string path;
            const auto path_without_name = names | std::views::drop(1);
            path.reserve(std::ranges::fold_left(path_without_name, path_without_name.size(), [](const size_t sum, const std::string& name)
            {
                return sum + name.size();
            }));

            if (!path_without_name.empty())
            {
                path.append(path_without_name.back());
                for (const auto& name : path_without_name | std::views::reverse)
                {
                    path += '\\';
                    path += name;
                }
            }

            sqlite3_result_text(ctx, path.c_str(), static_cast<int>(path.size()), SQLITE_TRANSIENT);
        }
        catch (...)
        {
            sqlite3_result_error(ctx, "Failed to execute UDF", -1);
        }
    }
}
