#include "stdafx.h"
#include "RegistryFetcher.hpp"

RegistryFetcher::RegistryFetcher(QObject* parent)
    : QObject(nullptr),
      _thread(std::make_unique<QThread>(this))
{
    if (parent)
        throw std::invalid_argument("parent must be nullptr");

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

void RegistryFetcher::fetch_count(const QString& query,
                                  const anyreg::SortColumn sort_column,
                                  const anyreg::SortOrder sort_order,
                                  const std::stop_token& stop_token) const
{
    if (stop_token.stop_requested())
    {
        return;
    }

    size_t count = 0;
    if (query.isEmpty())
    {
        const auto empty_query = _db.get_empty_query(sort_column, sort_order);
        count = empty_query.get_row_count();
    }
    else if (query.size() < 3)
    {
        const auto like_query = _db.get_like_query(sort_column, sort_order);
        count = like_query.get_row_count();
    }
    else
    {
        const auto fts_query = _db.get_fts_query(sort_column, sort_order);
        count = fts_query.get_row_count();
    }

    if (stop_token.stop_requested())
        return;

    emit count_fetched(count, stop_token);
}
