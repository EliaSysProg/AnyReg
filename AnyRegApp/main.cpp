#include "stdafx.h"

#include "AnyRegApp.hpp"
#include "AnyRegCore/Debug.hpp"

#include <stacktrace>
#include <QtWidgets/QApplication>

// static void log_stack_trace(const std::stacktrace& stacktrace)
// {
//     for (const auto& entry : stacktrace)
//     {
//         if (entry.source_file().empty()) continue;
//         qDebug() << std::format("{}:{}", entry.source_file(), entry.source_line());
//     }
// }

int main(int argc, char* argv[])
{
    try
    {
        TRACE("Starting application");
        QApplication a(argc, argv);
        AnyRegApp w;
        w.show();
        const auto ret = QApplication::exec();
        TRACE("Application finished gracefully");
        return ret;
    }
    catch (const std::exception& e)
    {
        TRACE("Error: {}", e.what());
    }
    catch (...)
    {
        TRACE("An unknown exception occured!");
    }

    return EXIT_FAILURE;
}
