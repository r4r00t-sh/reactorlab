import QtQuick
import QtQuick.Controls
import ReactorLab.Proto

Rectangle {
    id: root
    property var engine: null
    height: Theme.transportHeight
    color: Theme.bg1
    border.color: Theme.stroke

    Row {
        anchors.left: parent.left
        anchors.leftMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space2

        Button {
            text: "▶"
            onClicked: if (engine) engine.play()
            background: Rectangle { radius: Theme.radiusCtl; color: Theme.safe }
            contentItem: Text {
                text: parent.text; color: "#fff"; font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }
        Button {
            text: "❚❚"
            onClicked: if (engine) engine.pause()
            background: Rectangle { radius: Theme.radiusCtl; color: Theme.bg2; border.color: Theme.stroke }
            contentItem: Text {
                text: parent.text; color: Theme.textPrimary; font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }
        Button {
            text: "SCRAM"
            onClicked: if (engine) engine.scram()
            background: Rectangle { radius: Theme.radiusCtl; color: Theme.alarm }
            contentItem: Text {
                text: parent.text; color: "#fff"; font.pixelSize: Theme.typeSm; font.weight: Font.Bold
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }
        Button {
            text: "Reset"
            onClicked: if (engine) engine.reset()
            background: Rectangle { radius: Theme.radiusCtl; color: Theme.bg2; border.color: Theme.stroke }
            contentItem: Text {
                text: parent.text; color: Theme.textPrimary; font.pixelSize: Theme.typeSm
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: "Speed"
            color: Theme.textMuted
            font.pixelSize: Theme.typeSm
        }
        Slider {
            id: speed
            from: 0.25; to: 8; value: engine ? engine.speed : 1
            width: 120
            onMoved: if (engine) engine.speed = value
        }
    }

    // Timeline scrubber (demo)
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 420
        anchors.rightMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        height: 28
        radius: 8
        color: Theme.bg2
        border.color: Theme.stroke

        Rectangle {
            width: Math.min(parent.width, (engine ? engine.simTime : 0) / 120 * parent.width)
            height: parent.height
            radius: 8
            color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.35)
        }

        // event markers (fake)
        Repeater {
            model: [12, 40, 75]
            Rectangle {
                x: modelData / 120 * parent.width
                width: 2
                height: parent.height
                color: Theme.warn
            }
        }

        Text {
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            text: "timeline (demo)"
            color: Theme.textMuted
            font.pixelSize: Theme.typeXs
        }
    }
}
