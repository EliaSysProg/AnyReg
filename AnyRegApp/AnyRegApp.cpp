#include "stdafx.h"
#include "AnyRegApp.hpp"

#include <chrono>

using namespace std::chrono_literals;

AnyRegApp::AnyRegApp(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    installEventFilter(this);
    ui.searchBox->setFocus();

    db.index(HKEY_CURRENT_USER);

    connect(ui.searchBox, &QLineEdit::textChanged, this, &AnyRegApp::on_search_text_changed);

    on_search_text_changed();
}

bool AnyRegApp::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        const QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if (key_event->key() == Qt::Key_Escape)
        {
            ui.searchBox->clear();
            ui.searchBox->setFocus();
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

void AnyRegApp::on_search_text_changed()
{
    const auto query = ui.searchBox->text().toStdWString();
    const auto& results = query.empty() ? db.keys() : db.find_key(query);
    QStringList items;
    for (const auto& entry : results)
    {
        items << QString::fromStdWString(std::format(L"{}{}{}", entry.path, entry.path.empty() ? L"" : L"\\", entry.name));
    }

    ui.resultList->clear();
    ui.resultList->addItems(items);
}
