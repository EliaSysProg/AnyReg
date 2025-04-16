import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "Registry Indexer"

Item {
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        TextField {
            id: searchBox
            placeholderText: "Search registry keys..."
            onAccepted: regBridge.search(text)
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: regBridge.results

            delegate: ItemDelegate {
                width: parent.width
                text: modelData
            }
        }
    }
}

}
