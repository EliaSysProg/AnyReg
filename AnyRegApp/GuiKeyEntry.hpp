#pragma once

#include "AnyRegCore/RegistryEntry.hpp"

struct GuiKeyEntry
{
    QString name;
    QString path;
    QDateTime last_write_time;

    GuiKeyEntry() = default;
    GuiKeyEntry(QString name, QString path, QDateTime last_write_time);
    GuiKeyEntry(const anyreg::RegistryKeyView& key);
};
