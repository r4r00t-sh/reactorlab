import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

Item {
    id: root
    property var clock: null

    Column {
        anchors.fill: parent
        spacing: Theme.space3

        Row {
            width: parent.width
            spacing: Theme.space3
            Text {
                text: "Alarms"
                color: Theme.textPrimary
                font.pixelSize: Theme.typeLg
                font.weight: Font.DemiBold
                anchors.verticalCenter: parent.verticalCenter
            }
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: masterLbl.implicitWidth + 16
                height: 26
                radius: Theme.radiusPill
                color: (root.clock && root.clock.masterAlarm) ? Theme.alarm : Theme.safe
                Text {
                    id: masterLbl
                    anchors.centerIn: parent
                    text: (root.clock && root.clock.masterAlarm)
                          ? ("MASTER · " + (root.clock.alarmUnacked || 0) + " unacked")
                          : "MASTER clear"
                    color: "#fff"
                    font.pixelSize: Theme.typeXs
                    font.weight: Font.DemiBold
                }
            }
            Item { width: 12; height: 1 }
            Button {
                text: "Acknowledge"
                onClicked: if (root.clock) root.clock.acknowledgeAlarms()
                background: Rectangle {
                    radius: Theme.radiusCtl
                    color: Theme.bg2
                    border.color: Theme.stroke
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    font.pixelSize: Theme.typeSm
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            text: "Educational thresholds only — not a safety system. Latched until acknowledged."
            color: Theme.warn
            font.pixelSize: Theme.typeXs
        }

        ListView {
            width: parent.width
            height: parent.height - 90
            clip: true
            spacing: Theme.space2
            model: root.clock ? root.clock.alarmMsgs.length : 0
            delegate: Rectangle {
                width: ListView.view.width
                height: 56
                radius: Theme.radiusCtl
                color: Theme.bgCard
                border.color: (root.clock.alarmSeverities[index] === "alarm")
                              ? Theme.alarm : Theme.warn
                border.width: 2
                Row {
                    anchors.fill: parent
                    anchors.margins: Theme.space3
                    spacing: Theme.space3
                    Rectangle {
                        width: 10; height: 10; radius: 2
                        anchors.verticalCenter: parent.verticalCenter
                        color: (root.clock.alarmSeverities[index] === "alarm")
                               ? Theme.alarm : Theme.warn
                    }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2
                        Text {
                            text: root.clock.alarmIds[index]
                            color: Theme.textMuted
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.typeXs
                        }
                        Text {
                            text: root.clock.alarmMsgs[index]
                            color: Theme.textPrimary
                            font.pixelSize: Theme.typeSm
                        }
                    }
                }
            }
            Text {
                anchors.centerIn: parent
                visible: !root.clock || root.clock.alarmMsgs.length === 0
                text: "No active alarms"
                color: Theme.safe
                font.pixelSize: Theme.typeMd
            }
        }
    }
}
