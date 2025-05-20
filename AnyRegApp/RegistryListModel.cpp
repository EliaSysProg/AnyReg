#include "stdafx.h"
#include "RegistryListModel.hpp"

using namespace std::chrono;

RegistryListModel::RegistryListModel(QObject* parent)
    : QAbstractTableModel(parent),
      _db(anyreg::RegistryDatabase::open_read()),
      _find_statement(_db, anyreg::SortColumn::PATH, anyreg::SortOrder::ASCENDING),
      _key_range(_db, {})
{
}

int RegistryListModel::rowCount(const QModelIndex& parent) const
{
    Q_ASSERT(!parent.isValid());

    return static_cast<int>(_key_range.size());
}

int RegistryListModel::columnCount(const QModelIndex& parent) const
{
    Q_ASSERT(!parent.isValid());

    return 3;
}

QVariant RegistryListModel::data(const QModelIndex& index, const int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (index.row() != _current_index)
        {
            _current_entry = GuiKeyEntry(_key_range[index.row()]);
            _current_index = index.row();
        }

        const auto& [name, path, last_write_time] = _current_entry;
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
    beginResetModel();
    _find_statement.bind(query.toLocal8Bit().toStdString());
    _key_range = _find_statement.find();
    _current_index = -1;
    endResetModel();
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
    _find_statement = anyreg::FindKeyStatement(_db, column, order);
    _find_statement.bind(_query);
    _key_range = _find_statement.find();
    _current_index = -1;
    endResetModel();
}
