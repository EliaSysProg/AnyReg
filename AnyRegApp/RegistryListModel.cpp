#include "stdafx.h"
#include "RegistryListModel.hpp"

#include <algorithm>
#include <ranges>

RegistryListModel::RegistryListModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

int RegistryListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(_all_entries.size());
}

int RegistryListModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return 2; // e.g., Full Path and Name
}

QVariant RegistryListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || static_cast<size_t>(index.row()) >= _all_entries.size())
        return {};

    const RegistryKeyEntry& entry = _all_entries[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case 0: return QString::fromStdString(entry.get_full_path());
        case 1: return QString::fromStdString(entry.name);
        default: return {};
        }
    }

    if (role == Qt::ToolTipRole)
    {
        return QString::fromStdString(entry.name);
    }

    return {};
}

QVariant RegistryListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0: return "Full Path";
        case 1: return "Name";
        default: return {};
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

void RegistryListModel::set_entries(std::vector<RegistryKeyEntry> entries)
{
    beginResetModel();
    _all_entries = std::move(entries);
    endResetModel();
}
