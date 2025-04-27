#pragma once

#include "ui_AnyRegApp.h"

#include <QtWidgets/QMainWindow>

class AnyRegApp final : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnyRegApp(QWidget* parent = nullptr);

public slots:
    void start_query(const QString& query);

signals:
    void query_requested(const QString& query, const std::stop_token& stop_token);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

    Ui::AnyRegAppClass _ui;
    std::stop_source _stop_source;
};
