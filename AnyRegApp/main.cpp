#include "stdafx.h"

#include "AnyRegApp.hpp"

#include <stacktrace>
#include <QtWidgets/QApplication>

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
        qDebug() << std::format("Error: {}\n{}", e.what(), std::stacktrace::current());
        return 1;
    }
    catch (...)
    {
        qDebug() << std::format("An unknown exception occured!\n{}", std::stacktrace::current());
        return 1;
    }
}
