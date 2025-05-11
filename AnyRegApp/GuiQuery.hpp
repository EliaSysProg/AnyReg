#pragma once

#include "AnyRegCore/RegistryEntry.hpp"

struct GuiQuery
{
    QString query;
    anyreg::SortColumn sort_column;
    anyreg::SortOrder sort_order;
};
