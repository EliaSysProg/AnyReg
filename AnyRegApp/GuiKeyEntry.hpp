#pragma once

#include "AnyRegCore/RegistryEntry.hpp"

#include <QtCore>

struct GuiKeyEntry
{
    QString name;
    QString path;
    QDateTime last_write_time;

    GuiKeyEntry() = default;
    GuiKeyEntry(QString name, QString path, QDateTime last_write_time);
    GuiKeyEntry(const anyreg::RegistryKeyFullView& key);
};
