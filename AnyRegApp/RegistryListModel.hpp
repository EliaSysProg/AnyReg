#pragma once

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

    void set_query(const QString& query, int sort_column, Qt::SortOrder sort_order);

private:
    struct QRegistryKeyEntry
    {
        QString name;
        QString path;
        QDateTime last_write_time;

        QRegistryKeyEntry(QString name, QString path, QDateTime last_write_time);
        QRegistryKeyEntry(const anyreg::RegistryKeyView& key);
    };

    void fetch(size_t offset, size_t limit) const;
    
    anyreg::RegistryDatabase _db;
    std::string _query;
    size_t _record_count;
    mutable std::vector<QRegistryKeyEntry> _entries;
    mutable size_t _offset;
    anyreg::FindKeyStatement::SortColumn _sort_column;
    anyreg::FindKeyStatement::SortOrder _sort_order;
};

