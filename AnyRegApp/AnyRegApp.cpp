#include "stdafx.h"
#include "AnyRegApp.hpp"

#include "RegistryListModel.hpp"

AnyRegApp::AnyRegApp(QWidget* parent)
    : QMainWindow(parent)
{
    _ui.setupUi(this);

    installEventFilter(this);
    const auto model = new RegistryListModel;
    _ui.resultList->setModel(model);

    connect(_ui.searchBox, &QLineEdit::textChanged, model, &RegistryListModel::set_query);

    _ui.searchBox->setFocus();
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
