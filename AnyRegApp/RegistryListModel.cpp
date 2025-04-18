#include "stdafx.h"
#include "RegistryListModel.hpp"

#include <ranges>

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

    return _entries.has_value();
}

void RegistryListModel::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid())
        return;

    std::vector<RegistryKeyEntry> new_entries;
    new_entries.reserve(_fetch_chunk_size);

    int items_fetched = 0;
    do
    {
        if (_current_entry == _entries.value().end())
        {
            _entries = {};
            _current_entry = {};
            break;
        }

        new_entries.push_back(*_current_entry);
        ++items_fetched;
        ++_current_entry;
    }
    while (items_fetched < _fetch_chunk_size);

    if (items_fetched == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(),
                    static_cast<int>(_visible_entries.size()),
                    static_cast<int>(_visible_entries.size() + items_fetched - 1));
    _visible_entries.append_range(new_entries);
    endInsertRows();
}

void RegistryListModel::set_entries(MatchesT entries)
{
    beginResetModel();
    _entries = entries;
    _current_entry = _entries.value().begin();
    _visible_entries.clear();
    endResetModel();

    // Trigger initial fetch
    if (canFetchMore(QModelIndex()))
        fetchMore(QModelIndex());
}
