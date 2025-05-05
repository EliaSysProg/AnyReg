#include "stdafx.h"
#include "RegistryListModel.hpp"

static constexpr size_t FETCH_SIZE = 1024;

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

    return 3;
}

QVariant RegistryListModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || static_cast<size_t>(index.row()) >= _entries.size())
        return {};

    const auto& [name, path, last_write_time] = _entries[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case 0: return name;
        case 1: return path;
        case 2: return last_write_time;
        default: return {};
        }
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

    auto new_entries = try_fetch_next(FETCH_SIZE);

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

void RegistryListModel::set_query(const QString& query, const int sort_column, const Qt::SortOrder sort_order)
{
    anyreg::FindKeyStatement::SortColumn column;
    switch (sort_column)
    {
    case 0:
        column = anyreg::FindKeyStatement::SortColumn::NAME;
        break;
    case 1:
        column = anyreg::FindKeyStatement::SortColumn::PATH;
        break;
    case 2:
        column = anyreg::FindKeyStatement::SortColumn::LAST_WRITE_TIME;
        break;
    default:
        column = anyreg::FindKeyStatement::SortColumn::NAME;
        break;
    }

    const anyreg::FindKeyStatement::SortOrder order = sort_order == Qt::AscendingOrder
                                                          ? anyreg::FindKeyStatement::SortOrder::ASCENDING
                                                          : anyreg::FindKeyStatement::SortOrder::DESCENDING;

    beginResetModel();
    _find_operation = _db.find_keys(query.toStdString(), column, order);
    _it = _find_operation.begin();
    _entries = try_fetch_next(FETCH_SIZE);
    endResetModel();
}

std::vector<RegistryListModel::QRegistryKeyEntry> RegistryListModel::try_fetch_next(const size_t count)
{
    using namespace std::chrono;

    std::vector<QRegistryKeyEntry> new_entries;
    new_entries.reserve(std::min(count, 0x1000ull));

    for (; new_entries.size() < count && _it != _find_operation.end(); ++_it)
    {
        new_entries.emplace_back(QString::fromLocal8Bit(_it->name),
                                 QString::fromLocal8Bit(_it->get_absolute_path()),
                                 QDateTime::fromStdTimePoint(time_point_cast<milliseconds>(clock_cast<system_clock>(_it->last_write_time))));
    }

    return new_entries;
}
