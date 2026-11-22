import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

Item {
    id: root
    property var clock: null

    Column {
        anchors.fill: parent
        spacing: Theme.space3

        Text {
            text: "Event log"
            color: Theme.textPrimary
            font.pixelSize: Theme.typeLg
            font.weight: Font.DemiBold
        }
        Text {
            text: "Newest at bottom · play / pause / scram / alarms / scenarios / export"
            color: Theme.textMuted
            font.pixelSize: Theme.typeXs
        }

        Rectangle {
            width: parent.width
            height: parent.height - 60
            radius: Theme.radiusCard
            color: Theme.bgCard
            border.color: Theme.stroke
            clip: true

            ListView {
                id: list
                anchors.fill: parent
                anchors.margins: Theme.space2
                model: root.clock ? root.clock.eventLog : []
                clip: true
                delegate: Text {
                    width: list.width
                    text: modelData
                    color: Theme.textPrimary
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.typeXs
                    wrapMode: Text.WrapAnywhere
                }
                onCountChanged: positionViewAtEnd()
            }
        }
    }
}
