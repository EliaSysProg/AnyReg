#pragma once

#include "ui_AnyRegApp.h"

#include <QtWidgets/QMainWindow>

class AnyRegApp final : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnyRegApp(QWidget* parent = nullptr);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

    Ui::AnyRegAppClass _ui;
};
