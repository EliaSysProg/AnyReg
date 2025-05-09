#include "stdafx.h"

#include "AnyRegApp.hpp"

#include <stacktrace>
#include <QtWidgets/QApplication>

static void log_stack_trace(const std::stacktrace& stacktrace)
{
    for (const auto& entry : stacktrace)
    {
        if (entry.source_file().empty()) continue;
        qDebug() << std::format("{}:{}", entry.source_file(), entry.source_line());
    }
}

int main(int argc, char* argv[])
{
    try
    {
        QApplication a(argc, argv);
        AnyRegApp w;
        w.show();
        return QApplication::exec();
    }
    catch (const sql::DatabaseError& e)
    {
        qDebug() << std::format("Database error: {}", e.what());
        log_stack_trace(e.stacktrace());
        return 1;
    }
    catch (const std::exception& e)
    {
        qDebug() << std::format("Error: {}", e.what());
        return 1;
    }
    catch (...)
    {
        qDebug() << std::format("An unknown exception occured!");
        return 1;
    }
}
