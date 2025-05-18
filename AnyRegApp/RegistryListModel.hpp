#pragma once

#include "AnyRegCore/AnyRegCore.hpp"

#include <QAbstractTableModel>

class RegistryListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit RegistryListModel(anyreg::RegistryDatabase db, QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void set_query(const QString& query);
    void set_sort_order(int sort_column, Qt::SortOrder sort_order);

private:
    anyreg::RegistryDatabase _db;
    std::string _query;
    std::vector<anyreg::RegistryId> _entries;
    std::ranges::subrange<decltype(_entries)::const_iterator> _visible;
};
