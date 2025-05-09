#include "stdafx.h"
#include "RegistryListModel.hpp"

using namespace std::chrono;

// static constexpr size_t FETCH_SIZE = 20;

RegistryListModel::RegistryListModel(QObject* parent)
    : QAbstractTableModel(parent),
      _db(SQLITE_OPEN_READONLY),
      _record_count(0),
      _offset(0),
      _sort_column(),
      _sort_order()
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
        fetch(row >= 20 ? row - 20 : 0, 40);
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

void RegistryListModel::set_query(const QString& query, const int sort_column, const Qt::SortOrder sort_order)
{
    beginResetModel();

    switch (sort_column)
    {
    case 0:
        _sort_column = anyreg::FindKeyStatement::SortColumn::NAME;
        break;
    case 1:
        _sort_column = anyreg::FindKeyStatement::SortColumn::PATH;
        break;
    case 2:
        _sort_column = anyreg::FindKeyStatement::SortColumn::LAST_WRITE_TIME;
        break;
    default:
        _sort_column = anyreg::FindKeyStatement::SortColumn::NAME;
        break;
    }

    _sort_order = sort_order == Qt::AscendingOrder
                      ? anyreg::FindKeyStatement::SortOrder::ASCENDING
                      : anyreg::FindKeyStatement::SortOrder::DESCENDING;

    _query = query.toStdString();
    _record_count = _db.get_key_count(_query, _sort_column, _sort_order);
    qDebug() << std::format("Got {} results for query: {}", _record_count, _query);

    fetch(0, 40);
    endResetModel();
}

RegistryListModel::QRegistryKeyEntry::QRegistryKeyEntry(QString name, QString path, QDateTime last_write_time)
    : name(std::move(name)),
      path(std::move(path)),
      last_write_time(std::move(last_write_time))
{
}

RegistryListModel::QRegistryKeyEntry::QRegistryKeyEntry(const anyreg::RegistryKeyView& key)
    : name(QString::fromLocal8Bit(key.name)),
      path(QString::fromLocal8Bit(key.get_absolute_path())),
      last_write_time(QDateTime::fromStdTimePoint(time_point_cast<milliseconds>(clock_cast<system_clock>(key.last_write_time))))
{
}

void RegistryListModel::fetch(const size_t offset, const size_t limit) const
{
    _offset = offset;
    _entries.clear();
    const auto range = _db.find_keys(_query, _sort_column, _sort_order, offset, limit);
    for (const auto& key : range)
    {
        _entries.emplace_back(key);
    }
}
