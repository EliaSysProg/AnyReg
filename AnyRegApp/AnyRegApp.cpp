#include "stdafx.h"
#include "AnyRegApp.hpp"

AnyRegApp::AnyRegApp(QWidget* parent)
    : QMainWindow(parent)
{
    _ui.setupUi(this);
    installEventFilter(this);
    _ui.searchBox->setFocus();
    _registry_model = new RegistryListModel(this);
    _ui.resultList->setModel(_registry_model);

    on_search_text_changed(""); // Everything

    connect(_ui.searchBox, &QLineEdit::textChanged, this, &AnyRegApp::on_search_text_changed);
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

void AnyRegApp::on_search_text_changed(const QString& query)
{
    _registry_model->set_entries(_db.find_keys(query.toStdString()));
}
