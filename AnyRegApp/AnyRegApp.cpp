#include "stdafx.h"
#include "AnyRegApp.hpp"

#include "RegistryListModel.hpp"

AnyRegApp::AnyRegApp(QWidget* parent)
    : QMainWindow(parent)
{
    _ui.setupUi(this);

    installEventFilter(this);
    _model = new RegistryListModel;
    _fetcher = new RegistryFetcher;
    _ui.resultList->setModel(_model);

    _ui.searchBox->setFocus();
    _ui.resultList->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);
    _ui.resultList->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QFont bold_font = _ui.resultList->horizontalHeader()->font();
    bold_font.setBold(true);
    _ui.resultList->horizontalHeader()->setFont(bold_font);

    connect(_ui.searchBox, &QLineEdit::textChanged, _model, &RegistryListModel::set_query);
    connect(_ui.resultList->horizontalHeader(), &QHeaderView::sortIndicatorChanged, _model, &RegistryListModel::set_sort_order);
    connect(_model, &RegistryListModel::request_count, _fetcher, &RegistryFetcher::fetch_count, Qt::QueuedConnection);
    connect(_fetcher, &RegistryFetcher::count_fetched, _model, &RegistryListModel::set_count, Qt::QueuedConnection);

    set_table_query("");
}

void AnyRegApp::set_table_query(const QString& query)
{
    const auto column_index = _ui.resultList->horizontalHeader()->sortIndicatorSection();
    const auto sort_order = _ui.resultList->horizontalHeader()->sortIndicatorOrder();
    _model->set_sort_order(column_index, sort_order);
    _model->set_query(query);
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
