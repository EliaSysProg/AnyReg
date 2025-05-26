#include "stdafx.h"
#include "RegistryFetcher.hpp"

RegistryFetcher::RegistryFetcher()
    : QObject(nullptr),
      _thread(std::make_unique<QThread>(this)),
      _db(anyreg::RegistryDatabase::open_read()),
      _find_statement(_db, anyreg::SortColumn::PATH, anyreg::SortOrder::ASCENDING)
{
    moveToThread(_thread.get());
    _thread->setObjectName("RegistryFetcherThread");
    _thread->start();
}

RegistryFetcher::~RegistryFetcher()
{
    QMetaObject::invokeMethod(this, "cleanup");
    _thread->quit();
    _thread->wait();
}

void RegistryFetcher::set_order(anyreg::SortColumn sort_column, anyreg::SortOrder sort_order)
{
    _find_statement = _db.find_keys(sort_column, sort_order);
    _find_statement.bind(_query);
}

void RegistryFetcher::set_query(const std::string& query)
{
    _query = query;
    _find_statement.bind(_query);
}

void RegistryFetcher::fetch()
{
    emit result_ready(_find_statement.find());
}
