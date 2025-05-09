#pragma once

#include "GuiKeyEntry.hpp"
#include "GuiQuery.hpp"
#include "AnyRegCore/RegistryDatabase.hpp"

#include <QAbstractTableModel>

class RegistryListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit RegistryListModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void set_query(const QString& query);
    void set_sort_order(int sort_column, Qt::SortOrder sort_order);

public slots:
    void set_count(size_t count, const std::stop_token& stop_token);

signals:
    void request_count(const QString& query,
                       anyreg::FindKeyStatement::SortColumn sort_column,
                       anyreg::FindKeyStatement::SortOrder sort_order,
                       const std::stop_token& stop_token) const;

private:
    void fetch(size_t offset, size_t limit) const;

    anyreg::RegistryDatabase _db;
    GuiQuery _current_query{};
    GuiQuery _next_query{};
    mutable std::vector<GuiKeyEntry> _entries;
    mutable size_t _offset{};
    size_t _record_count{};
    std::stop_source _fetch_stop_source;
};
