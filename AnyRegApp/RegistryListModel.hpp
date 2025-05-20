#pragma once

#include "GuiKeyEntry.hpp"
#include "GuiQuery.hpp"
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

    void set_query(const QString& query);
    void set_sort_order(int sort_column, Qt::SortOrder sort_order);

private:
    anyreg::RegistryDatabase _db;
    std::string _query;
    anyreg::FindKeyStatement _find_statement;
    anyreg::RegistryRecordRange _key_range;
    mutable GuiKeyEntry _current_entry;
    mutable int64_t _current_index = -1;
};
