#pragma once

#include "AnyRegCore/AnyRegDbConnection.hpp" // Your native indexer

#include <QObject>
#include <QStringListModel>

class RegBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList results READ results NOTIFY resultsChanged)

public:
    explicit RegBridge(QObject* parent = nullptr);

    QStringList results() const;

    Q_INVOKABLE void search(const QString& query);

signals:
    void resultsChanged();

private:
    AnyRegDbConnection _db;
    QStringListModel _model;
};
