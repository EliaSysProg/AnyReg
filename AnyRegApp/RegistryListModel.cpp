#include "stdafx.h"
#include "RegistryListModel.hpp"

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
    return 3; // e.g., Full Path and Name
}

QVariant RegistryListModel::data(const QModelIndex& index, const int role) const
{
    using namespace std::chrono;
    if (!index.isValid() || static_cast<size_t>(index.row()) >= _all_entries.size())
        return {};

    const auto& [name, path, last_write_time] = _all_entries[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case 0: return QString::fromStdString(name);
        case 1: return QString::fromStdString(path);
        case 2: return QDateTime::fromStdTimePoint(
                time_point_cast<milliseconds>(clock_cast<system_clock>(last_write_time)));
        default: return {};
        }
    }

    if (role == Qt::ToolTipRole)
    {
        return QString::fromStdString(name);
    }

    return {};
}

QVariant RegistryListModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0: return "Name";
        case 1: return "Path";
        case 2: return "Date Modified";
        default: return {};
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

void RegistryListModel::set_entries(std::vector<anyreg::RegistryKeyEntry> entries)
{
    beginResetModel();
    _all_entries = std::move(entries);
    endResetModel();
}
