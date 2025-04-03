#include "Pch.hpp"
#include "AnyRegDB.hpp"

AnyRegDB::AnyRegDB(std::string filename)
    : _db(std::move(filename))
{
}
