#include "stdafx.h"
#include "RegistryListModel.hpp"

#include "AnyRegCore/Debug.hpp"

#include <algorithm>

using namespace std::chrono;

RegistryListModel::RegistryListModel(anyreg::RegistryDatabase db, QObject* parent)
    : QAbstractTableModel(parent),
      _db(std::move(db))
{
    _entries.assign_range(_db | std::views::keys);
    _visible = _entries;
    TRACE("Initial keys count: {}", _entries.size());
}

int RegistryListModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_visible.size());
}

int RegistryListModel::columnCount(const QModelIndex&) const
{
    return 3;
}

QVariant RegistryListModel::data(const QModelIndex& index, const int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto id = _visible[index.row()];
        const auto& entry = _db.at(id);

        switch (index.column())
        {
        case 0: return QString::fromLocal8Bit(entry.name);
        case 1: return QString::fromLocal8Bit(anyreg::key_full_path(_db, entry.parent_id));
        case 2: return QDateTime::fromStdTimePoint(time_point_cast<milliseconds>(clock_cast<system_clock>(entry.last_write_time)));
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
    static constexpr auto ichar_equals = [](const char a, const char b) -> bool
    {
        return std::tolower(static_cast<unsigned char>(a)) ==
            std::tolower(static_cast<unsigned char>(b));
    };

    beginResetModel();

    _query = query.toLocal8Bit().toStdString();
    TRACE("Query: {}", _query);

    const auto pivot = std::stable_partition(_entries.begin(), _entries.end(), [&](const anyreg::RegistryId& id)
    {
        const auto& entry = _db.at(id);
        return std::search(entry.name.begin(), entry.name.end(),
                           _query.begin(), _query.end(),
                           ichar_equals) != entry.name.end();
    });

    _visible = {_entries.begin(), pivot};

    endResetModel();

    TRACE("Visible keys count: {}", _visible.size());
}

void RegistryListModel::set_sort_order(const int /*sort_column*/, const Qt::SortOrder /*sort_order*/)
{
    // anyreg::SortColumn column;
    // switch (sort_column)
    // {
    // case 0:
    //     column = anyreg::SortColumn::NAME;
    //     break;
    // case 1:
    //     column = anyreg::SortColumn::PATH;
    //     break;
    // case 2:
    //     column = anyreg::SortColumn::LAST_WRITE_TIME;
    //     break;
    // default:
    //     column = anyreg::SortColumn::NAME;
    //     break;
    // }
    //
    // const auto order = sort_order == Qt::AscendingOrder
    //                        ? anyreg::SortOrder::ASCENDING
    //                        : anyreg::SortOrder::DESCENDING;
    //
    // beginResetModel();
    // endResetModel();
}
