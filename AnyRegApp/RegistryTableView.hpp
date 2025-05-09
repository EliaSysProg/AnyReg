#pragma once

#include <QTableView>

class RegistryTableView  : public QTableView
{
    Q_OBJECT

public:
    explicit RegistryTableView(QWidget *parent);

    // Disable keyboard search
    void keyboardSearch(const QString&) override {}
};
