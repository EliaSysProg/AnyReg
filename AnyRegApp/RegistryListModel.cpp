#include "stdafx.h"
#include "RegistryListModel.hpp"

RegistryListModel::RegistryListModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int RegistryListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(_entries.size());
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
    if (!index.isValid() || static_cast<size_t>(index.row()) >= _entries.size())
        return {};

    const auto& [name, path, last_write_time] = _entries[index.row()];

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


void RegistryListModel::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid())
        return;

    constexpr int batch_size = 100; // Or any batch size you prefer

    std::vector<anyreg::RegistryKeyEntry> new_entries;
    new_entries.reserve(batch_size);

    for (; new_entries.size() < batch_size && _it != _find_operation.end(); ++_it)
    {
        new_entries.emplace_back(std::string(_it->name), std::string(_it->path), _it->last_write_time);
    }

    beginInsertRows(QModelIndex(), static_cast<int>(_entries.size()), static_cast<int>(_entries.size() + new_entries.size() - 1));
    _entries.append_range(std::move(new_entries));
    endInsertRows();
}

bool RegistryListModel::canFetchMore(const QModelIndex& parent) const
{
    if (parent.isValid())
        return false;

    return _it != _find_operation.end();
}

void RegistryListModel::set_query(const QString& query)
{
    beginResetModel();
    _find_operation = _db.find_keys(query.toStdString());
    _it = _find_operation.begin();
    _entries.clear();
    endResetModel();
    if (canFetchMore({})) fetchMore({});
}
