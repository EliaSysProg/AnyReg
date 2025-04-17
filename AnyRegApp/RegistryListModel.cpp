#include "stdafx.h"
#include "RegistryListModel.hpp"

RegistryListModel::RegistryListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int RegistryListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(_visible_entries.size());
}

QVariant RegistryListModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || static_cast<size_t>(index.row()) >= _visible_entries.size())
        return {};

    const RegistryKeyEntry& entry = _visible_entries[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdWString(entry.get_full_path());
    case Qt::ToolTipRole:
        return QString::fromStdWString(entry.name);
    default:
        return {};
    }
}

bool RegistryListModel::canFetchMore(const QModelIndex& parent) const
{
    if (parent.isValid())
        return false;

    return _visible_entries.size() < _all_entries.size();
}

void RegistryListModel::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid())
        return;

    int remaining = static_cast<int>(_all_entries.size() - _visible_entries.size());
    int items_to_fetch = std::min(_fetch_chunk_size, remaining);

    if (items_to_fetch > 0)
    {
        beginInsertRows(QModelIndex(),
                        static_cast<int>(_visible_entries.size()),
                        static_cast<int>(_visible_entries.size() + items_to_fetch - 1));

        auto start = _all_entries.begin() + _visible_entries.size();
        auto end = start + items_to_fetch;
        _visible_entries.insert(_visible_entries.end(), start, end);

        endInsertRows();
    }
}

void RegistryListModel::set_entries(const std::vector<RegistryKeyEntry>& entries)
{
    beginResetModel();
    _all_entries = entries;
    _visible_entries.clear();
    endResetModel();

    // Trigger initial fetch
    if (canFetchMore(QModelIndex()))
        fetchMore(QModelIndex());
}

void RegistryListModel::refresh()
{
    beginResetModel();
    endResetModel();
}
