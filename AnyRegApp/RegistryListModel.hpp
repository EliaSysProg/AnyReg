#pragma once

#include "AnyRegCore/RegistryEntry.hpp"

#include <QAbstractListModel>

class RegistryListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit RegistryListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool canFetchMore(const QModelIndex& parent) const override;
    void fetchMore(const QModelIndex& parent) override;

    void set_entries(const std::vector<RegistryKeyEntry>& entries);
    void refresh();

private:
    std::vector<RegistryKeyEntry> _all_entries;
    std::vector<RegistryKeyEntry> _visible_entries;

    int _fetch_chunk_size = 100;
};
