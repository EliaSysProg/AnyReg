#include "stdafx.h"
#include "AnyRegApp.hpp"

#include "RegistryListModel.hpp"

AnyRegApp::AnyRegApp(QWidget* parent)
    : QMainWindow(parent)
{
    _ui.setupUi(this);

    installEventFilter(this);
    _model = new RegistryListModel;
    _ui.resultList->setModel(_model);

    connect(_ui.searchBox, &QLineEdit::textChanged, this, &AnyRegApp::set_table_query);

    _ui.searchBox->setFocus();
    _ui.resultList->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
}

void AnyRegApp::set_table_query(const QString& query)
{
    const auto column_index = _ui.resultList->horizontalHeader()->sortIndicatorSection();
    const auto sort_order = _ui.resultList->horizontalHeader()->sortIndicatorOrder();
    _model->set_query(query, column_index, sort_order);
}

bool AnyRegApp::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        const QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if (key_event->key() == Qt::Key_Escape)
        {
            _ui.searchBox->clear();
            _ui.searchBox->setFocus();
            return true;
        }
        if (key_event->key() == Qt::Key_Q && key_event->modifiers() == Qt::ControlModifier)
        {
            close();
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}
