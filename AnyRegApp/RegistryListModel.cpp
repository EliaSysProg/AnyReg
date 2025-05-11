#include "stdafx.h"
#include "RegistryListModel.hpp"

using namespace std::chrono;

static constexpr size_t FETCH_SIZE = 40;

RegistryListModel::RegistryListModel(QObject* parent)
    : QAbstractTableModel(parent),
      _empty_statement(_db.get_empty_query(anyreg::SortColumn::PATH, anyreg::SortOrder::ASCENDING)),
      _like_statement(_db.get_like_query(anyreg::SortColumn::PATH, anyreg::SortOrder::ASCENDING)),
      _fts_statement(_db.get_fts_query(anyreg::SortColumn::PATH, anyreg::SortOrder::ASCENDING))

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
        fetch(row, FETCH_SIZE);
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
    _next_query.query = query.toLocal8Bit();
    request_count(_next_query.query, _next_query.sort_column, _next_query.sort_order, _fetch_stop_source.get_token());
}

void RegistryListModel::set_sort_order(const int sort_column, const Qt::SortOrder sort_order)
{
    anyreg::SortColumn column;
    switch (sort_column)
    {
    case 0:
        column = anyreg::SortColumn::NAME;
        break;
    case 1:
        column = anyreg::SortColumn::PATH;
        break;
    case 2:
        column = anyreg::SortColumn::LAST_WRITE_TIME;
        break;
    default:
        column = anyreg::SortColumn::NAME;
        break;
    }

    const auto order = sort_order == Qt::AscendingOrder
                           ? anyreg::SortOrder::ASCENDING
                           : anyreg::SortOrder::DESCENDING;

    beginResetModel();
    _current_query.sort_column = column;
    _current_query.sort_order = order;
    _entries.clear();

    if (_current_query.query.isEmpty())
    {
        _empty_statement = _db.get_empty_query(_current_query.sort_column, _current_query.sort_order);
    }
    else if (_current_query.query.size() < 3)
    {
        _like_statement = _db.get_like_query(_current_query.sort_column, _current_query.sort_order);
        _like_statement.bind_query(_current_query.query.toStdString());
    }
    else
    {
        _fts_statement = _db.get_fts_query(_current_query.sort_column, _current_query.sort_order);
        _fts_statement.bind_query(_current_query.query.toStdString());
    }
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

    if (_current_query.query.isEmpty())
    {
        _empty_statement.reset();
    }
    else if (_current_query.query.size() < 3)
    {
        _like_statement.reset();
        _like_statement.bind_query(_current_query.query.toStdString());
    }
    else
    {
        _fts_statement.reset();
        _fts_statement.bind_query(_current_query.query.toStdString());
    }

    endResetModel();
}

void RegistryListModel::fetch(const size_t offset, const size_t limit) const
{
    qDebug() << std::format("Fetching {}-{} from {}", offset, offset + limit, _current_query.query.toStdString());
    _offset = offset;
    _entries.clear();
    auto insert_range = [this](const anyreg::RegistryKeyView& key) { _entries.emplace_back(key); };
    if (_current_query.query.isEmpty())
    {
        _empty_statement.reset();
        _empty_statement.bind_range(offset, limit);
        std::for_each(_empty_statement.begin(), _empty_statement.end(), insert_range);
    }
    else if (_current_query.query.size() < 3)
    {
        _like_statement.reset();
        _like_statement.bind_range(offset, limit);
        std::for_each(_like_statement.begin(), _like_statement.end(), insert_range);
    }
    else
    {
        _fts_statement.reset();
        _fts_statement.bind_range(offset, limit);
        std::for_each(_fts_statement.begin(), _fts_statement.end(), insert_range);
    }

    qDebug() << std::format("Fetched {} entries", _entries.size());
}
