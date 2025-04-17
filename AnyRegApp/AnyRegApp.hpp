#pragma once

#include "ui_AnyRegApp.h"
#include "AnyRegCore/AnyRegDbConnection.hpp" // Native class

#include <QTimer>
#include <QtWidgets/QMainWindow>

class AnyRegApp : public QMainWindow
{
    Q_OBJECT

public:
    AnyRegApp(QWidget* parent = nullptr);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSearchTextChanged();
    void handleDebouncedSearch();

private:
    Ui::AnyRegAppClass ui;
    AnyRegDbConnection db;
    QTimer cooldown_timer;
};
