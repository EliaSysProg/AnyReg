#pragma once

#include "AnyRegCore/AnyRegCore.hpp"

#include <QObject>

class RegistryFetcher final : public QObject
{
    Q_OBJECT

public:
    RegistryFetcher();
    ~RegistryFetcher() override;

public slots:
    void set_order(anyreg::SortColumn sort_column, anyreg::SortOrder sort_order);
    void set_query(const std::string& query);
    void fetch();

signals:
    void result_ready(anyreg::RegistryRecordRange range);

private:
    std::unique_ptr<QThread> _thread;
    anyreg::RegistryDatabase _db;
    std::string _query;
    anyreg::FindKeyStatement _find_statement;
};
