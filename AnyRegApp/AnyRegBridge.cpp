#include "AnyRegBridge.hpp"

RegBridge::RegBridge(QObject* parent)
    : QObject(parent)
{
    _db.index(HKEY_CURRENT_USER); // or another root
}

QStringList RegBridge::results() const {
    return _model.stringList();
}

void RegBridge::search(const QString& query) {
    QStringList result;
    for (const auto& entry : _db.find_key(query.toStdWString())) {
        result << QString::fromStdWString(entry.path + L'\\' + entry.name);
    }

    _model.setStringList(result);
    emit resultsChanged();
}
