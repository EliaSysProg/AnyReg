#pragma once

#include "AnyRegCore/RegistryDatabase.hpp"

class RegistryQueryController final : public QObject
{
    Q_OBJECT

public:
    explicit RegistryQueryController(QObject* parent = nullptr);
    ~RegistryQueryController() override;

public slots:
    void start_query(const QString& query, const std::stop_token& stop_token);

signals:
    void query_finished(std::vector<anyreg::RegistryKeyEntry> entries);

private:
    std::unique_ptr<QThread> _thread;
    anyreg::RegistryDatabase _db;
};
