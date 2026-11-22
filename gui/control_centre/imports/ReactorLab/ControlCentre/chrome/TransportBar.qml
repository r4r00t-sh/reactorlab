import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

Rectangle {
    id: root
    property var clock: null
    height: Theme.transportHeight
    color: Theme.bg1
    border.color: Theme.stroke

    Row {
        anchors.left: parent.left
        anchors.leftMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space2

        Button {
            text: "Play"
            onClicked: if (clock) clock.play()
            background: Rectangle { radius: Theme.radiusCtl; color: Theme.safe }
            contentItem: Text {
                text: parent.text; color: "#fff"; font.pixelSize: Theme.typeSm; font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }
        Button {
            text: "Pause"
            onClicked: if (clock) clock.pause()
            background: Rectangle { radius: Theme.radiusCtl; color: Theme.bg2; border.color: Theme.stroke }
            contentItem: Text {
                text: parent.text; color: Theme.textPrimary; font.pixelSize: Theme.typeSm
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }
        Button {
            text: "SCRAM"
            onClicked: if (clock) clock.scram()
            background: Rectangle { radius: Theme.radiusCtl; color: Theme.alarm }
            contentItem: Text {
                text: parent.text; color: "#fff"; font.pixelSize: Theme.typeSm; font.weight: Font.Bold
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }
        Button {
            text: "Reset"
            onClicked: if (clock) clock.reset()
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
            from: 0.25; to: 8
            value: clock ? clock.speed : 1
            width: 140
            onMoved: if (clock) clock.speed = value
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 460
        anchors.rightMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        height: 30
        radius: 8
        color: Theme.bg2
        border.color: Theme.stroke

        Rectangle {
            width: {
                var tt = 0
                if (clock) tt = (clock.simTime !== undefined) ? clock.simTime : (clock.t || 0)
                return Math.min(parent.width, tt / 180 * parent.width)
            }
            height: parent.height
            radius: 8
            color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.35)
            Behavior on width { NumberAnimation { duration: Theme.durFast; easing.type: Easing.OutCubic } }
        }

        Repeater {
            model: [20, 60, 110]
            Rectangle {
                x: modelData / 180 * parent.width
                width: 2
                height: parent.height
                color: Theme.warn
            }
        }

        Text {
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            text: "timeline scrubber · UI-7 replay"
            color: Theme.textMuted
            font.pixelSize: Theme.typeXs
        }
    }
}
