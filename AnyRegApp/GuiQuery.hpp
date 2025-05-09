#pragma once

#include "AnyRegCore/FindKeyStatement.hpp"

struct GuiQuery
{
    QString query;
    anyreg::FindKeyStatement::SortColumn sort_column;
    anyreg::FindKeyStatement::SortOrder sort_order;
};
