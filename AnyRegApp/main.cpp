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
        return a.exec();
    }
    catch (const std::exception& e)
    {
        qDebug() << "Error: " << e.what();
        return 1;
    }
    catch (...)
    {
        qDebug() << "An unknown exception occured!";
        return 1;
    }
}
