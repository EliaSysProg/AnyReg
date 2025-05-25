#pragma once

#include "SQLite3/sqlite3.h"

namespace anyreg
{
    void full_path_udf(sqlite3_context* ctx, int argc, sqlite3_value** argv);
}
