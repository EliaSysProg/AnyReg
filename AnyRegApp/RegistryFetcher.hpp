#pragma once

#include "AnyRegCore/FindKeyStatement.hpp"
#include "AnyRegCore/RegistryDatabase.hpp"
#include "AnyRegCore/RegistryEntry.hpp"

#include <QObject>

class RegistryFetcher final : public QObject
{
    Q_OBJECT

public:
    explicit RegistryFetcher(QObject* parent = nullptr);
    ~RegistryFetcher() override;

public slots:
    void fetch_count(const QString& query,
                     anyreg::FindKeyStatement::SortColumn sort_column,
                     anyreg::FindKeyStatement::SortOrder sort_order) const;

signals:
    void count_fetched(size_t count) const;

private:
    std::unique_ptr<QThread> _thread;
    anyreg::RegistryDatabase _db;
};
