#include "stdafx.h"

#include "AnyRegApp.hpp"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AnyRegApp w;
    w.show();
    return a.exec();
}
