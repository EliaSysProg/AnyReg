#pragma once

#include "RegistryListModel.hpp"
#include "ui_AnyRegApp.h"
#include "AnyRegCore/RegistryDatabase.hpp" // Native class

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
    RegistryListModel* _registry_model;
    Ui::AnyRegAppClass _ui;
    RegistryDatabase _db;
};
