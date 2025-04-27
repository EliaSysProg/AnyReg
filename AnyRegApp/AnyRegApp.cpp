#include "stdafx.h"
#include "AnyRegApp.hpp"

#include "RegistryListModel.hpp"
#include "RegistryQueryController.hpp"

AnyRegApp::AnyRegApp(QWidget* parent)
    : QMainWindow(parent)
{
    _ui.setupUi(this);

    installEventFilter(this);
    const auto model = new RegistryListModel;
    _ui.resultList->setModel(model);

    const auto worker = new RegistryQueryController;

    connect(_ui.searchBox, &QLineEdit::textChanged, this, &AnyRegApp::start_query);
    connect(this ,&AnyRegApp::query_requested, worker, &RegistryQueryController::start_query);
    connect(worker, &RegistryQueryController::query_finished, model, &RegistryListModel::set_entries);

    start_query("");

    _ui.searchBox->setFocus();
}

void AnyRegApp::start_query(const QString& query)
{
    _stop_source.request_stop();
    _stop_source = {};
    emit query_requested(query, _stop_source.get_token());
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
