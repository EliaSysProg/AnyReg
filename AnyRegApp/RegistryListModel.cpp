#include "stdafx.h"
#include "RegistryListModel.hpp"

using namespace std::chrono;

static constexpr size_t FETCH_SIZE = 40;

RegistryListModel::RegistryListModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int RegistryListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(_record_count);
}

int RegistryListModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return 3;
}

QVariant RegistryListModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return {};

    const auto row = static_cast<size_t>(index.row());

    if (row < _offset || row >= _offset + _entries.size())
    {
        fetch(row >= 20 ? row - 20 : 0, FETCH_SIZE);
    }

    const auto& [name, path, last_write_time] = _entries.at(row - _offset);

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

void RegistryListModel::set_query(const QString& query)
{
    _fetch_stop_source.request_stop();
    _fetch_stop_source = {};
    _next_query = _current_query;
    _next_query.query = query;
    request_count(_next_query.query, _next_query.sort_column, _next_query.sort_order, _fetch_stop_source.get_token());
}

void RegistryListModel::set_sort_order(const int sort_column, const Qt::SortOrder sort_order)
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

    const auto order = sort_order == Qt::AscendingOrder
                           ? anyreg::FindKeyStatement::SortOrder::ASCENDING
                           : anyreg::FindKeyStatement::SortOrder::DESCENDING;

    beginResetModel();
    _current_query.sort_column = column;
    _current_query.sort_order = order;
    _entries.clear();
    endResetModel();
}

void RegistryListModel::set_count(const size_t count, const std::stop_token& stop_token)
{
    if (stop_token.stop_requested())
        return;

    beginResetModel();
    _record_count = count;
    _current_query = std::exchange(_next_query, {});
    _entries.clear();
    endResetModel();
}

void RegistryListModel::fetch(const size_t offset, const size_t limit) const
{
    qDebug() << std::format("Fetching {}-{} from {}", offset, offset + limit, _current_query.query.toStdString());
    _offset = offset;
    _entries.clear();
    const auto range = _db.find_keys(_current_query.query.toStdString(),
                                     _current_query.sort_column,
                                     _current_query.sort_order,
                                     offset,
                                     limit);

    for (const auto& key : range)
    {
        _entries.emplace_back(key);
    }
}
