#pragma once

#include "RegistryListModel.hpp"
#include "ui_AnyRegApp.h"

#include <QtWidgets/QMainWindow>

class AnyRegApp final : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnyRegApp(QWidget* parent = nullptr);

private slots:
    void set_table_query(const QString& query);
    void set_table_sort(int index, Qt::SortOrder order);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

    Ui::AnyRegAppClass _ui;
    RegistryListModel* _model = nullptr;
};
