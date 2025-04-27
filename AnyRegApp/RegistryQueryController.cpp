#include "stdafx.h"
#include "RegistryQueryController.hpp"

RegistryQueryController::RegistryQueryController(QObject* parent)
    : QObject(parent),
_thread(std::make_unique<QThread>())
{
    moveToThread(_thread.get());
    _thread->setObjectName("DatabaseWorkerThread");
    _thread->start();
}

RegistryQueryController::~RegistryQueryController()
{
    QMetaObject::invokeMethod(this, "cleanup");
    _thread->quit();
    _thread->wait();
}

void RegistryQueryController::start_query(const QString& query, const std::stop_token& stop_token)
{
    if (stop_token.stop_requested()) { return; }
    auto keys = _db.find_keys(query.toStdString(), stop_token);
    if (stop_token.stop_requested()) { return; }
    emit query_finished(std::move(keys));
}
