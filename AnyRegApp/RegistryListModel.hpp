#pragma once

#include "AnyRegCore/RegistryDatabase.hpp"

#include <QAbstractTableModel>

class RegistryListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit RegistryListModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void fetchMore(const QModelIndex& parent) override;
    bool canFetchMore(const QModelIndex& parent) const override;

    void set_query(const QString& query);

private:
    anyreg::RegistryDatabase _db;
    anyreg::RegistryDatabase::FindKeyRange _find_operation = _db.find_keys("");
    anyreg::FindKeyStatement::iterator _it = _find_operation.begin();
    std::vector<anyreg::RegistryKeyEntry> _entries;
};

