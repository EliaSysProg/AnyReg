#pragma once

#include "ui_AnyRegApp.h"
#include "AnyRegCore/AnyRegDbConnection.hpp" // Native class

#include <QTimer>
#include <QtWidgets/QMainWindow>

class AnyRegApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnyRegApp(QWidget* parent = nullptr);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void on_search_text_changed();

private:
    Ui::AnyRegAppClass ui;
    AnyRegDbConnection db;
    QTimer cooldown_timer;
};
