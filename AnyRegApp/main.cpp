#include "stdafx.h"

#include "AnyRegApp.hpp"

#include <stacktrace>
#include <QtWidgets/QApplication>

void log_current_stack_trace()
{
    const auto stacktrace = std::stacktrace::current();
    for (const auto& frame : stacktrace)
    {
        qDebug() << frame.description();
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
    catch (const std::exception& e)
    {
        qDebug() << std::format("Error: {}", e.what());
        log_current_stack_trace();
        return 1;
    }
    catch (...)
    {
        qDebug() << std::format("An unknown exception occured!");
        log_current_stack_trace();
        return 1;
    }
}
