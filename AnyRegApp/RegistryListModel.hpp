#pragma once

#include "AnyRegCore/RegistryEntry.hpp"

#include <QAbstractTableModel>
#include <vector>

class RegistryListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit RegistryListModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void set_entries(std::vector<anyreg::RegistryKeyEntry> entries);

private:
    std::vector<anyreg::RegistryKeyEntry> _all_entries;
};
