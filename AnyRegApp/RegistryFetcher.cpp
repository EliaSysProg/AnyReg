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
                                  const anyreg::FindKeyStatement::SortColumn sort_column,
                                  const anyreg::FindKeyStatement::SortOrder sort_order,
                                  const std::stop_token& stop_token) const
{
    const auto count = _db.get_key_count(query.toStdString(), sort_column, sort_order);

    if (stop_token.stop_requested())
        return;

    emit count_fetched(count, stop_token);
}
