#pragma once

#include "AnyRegCore/RegistryDatabase.hpp"
#include "AnyRegCore/RegistryEntry.hpp"

#include <QAbstractListModel>
#include <optional>
#include <vector>

class RegistryListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    using MatchesT = decltype(std::declval<RegistryDatabase>().find_key(L""));
    using MatchesIteratorT = decltype(std::declval<MatchesT>().begin());
    
    explicit RegistryListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool canFetchMore(const QModelIndex& parent) const override;
    void fetchMore(const QModelIndex& parent) override;

    void set_entries(MatchesT entries);

private:
    std::optional<MatchesT> _entries;
    MatchesIteratorT _current_entry;
    std::vector<RegistryKeyEntry> _visible_entries;

    int _fetch_chunk_size = 100;
};
