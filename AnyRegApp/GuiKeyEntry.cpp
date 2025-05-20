#include "stdafx.h"
#include "GuiKeyEntry.hpp"

using namespace std::chrono;

GuiKeyEntry::GuiKeyEntry(QString name, QString path, QDateTime last_write_time)
    : name(std::move(name)),
      path(std::move(path)),
      last_write_time(std::move(last_write_time))
{
}

GuiKeyEntry::GuiKeyEntry(const anyreg::RegistryKeyView& key)
    : name(QString::fromLocal8Bit(key.name)),
      path(QString::fromLocal8Bit(std::to_string(key.parent_id))),
      last_write_time(QDateTime::fromStdTimePoint(time_point_cast<milliseconds>(clock_cast<system_clock>(key.last_write_time))))
{
}
