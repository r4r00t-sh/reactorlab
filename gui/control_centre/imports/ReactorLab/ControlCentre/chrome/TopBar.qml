import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

Rectangle {
    id: root
    property var clock: null
    height: Theme.topBarHeight
    color: Theme.bg1
    border.color: Theme.stroke

    Row {
        id: left
        anchors.left: parent.left
        anchors.leftMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space4

        Column {
            Text {
                text: clock ? clock.reactorName : "ReactorLab"
                color: Theme.textPrimary
                font.family: Theme.fontUi
                font.pixelSize: Theme.typeMd
                font.weight: Font.DemiBold
            }
            Text {
                text: "Control centre shell · educational / not safety-grade"
                color: Theme.textMuted
                font.pixelSize: Theme.typeXs
            }
        }

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: stateLbl.implicitWidth + 20
            height: 28
            radius: Theme.radiusPill
            color: {
                if (!clock) return Theme.bg2
                if (clock.simState === "Scram") return Theme.alarm
                if (clock.running) return Theme.safe
                return Theme.bg2
            }
            Text {
                id: stateLbl
                anchors.centerIn: parent
                text: clock ? clock.simState : "—"
                color: Theme.textPrimary
                font.pixelSize: Theme.typeSm
                font.weight: Font.Medium
            }
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: {
                if (!clock) return ""
                var tt = (clock.simTime !== undefined) ? clock.simTime : clock.t
                var sp = (clock.speed !== undefined) ? clock.speed : 1
                return "t = " + Number(tt).toFixed(1) + " s  ×" + Number(sp).toFixed(1)
            }
            color: Theme.textPrimary
            font.family: Theme.fontMono
            font.pixelSize: Theme.typeSm
        }

        // Layout presets
        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.space1
            Repeater {
                model: LayoutStore.presetNames
                delegate: Button {
                    text: modelData
                    flat: true
                    onClicked: LayoutStore.setPreset(index)
                    background: Rectangle {
                        radius: Theme.radiusCtl
                        color: LayoutStore.preset === index ? Theme.accentDim : Theme.bg2
                        border.color: LayoutStore.preset === index ? Theme.accent : Theme.stroke
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.textPrimary
                        font.pixelSize: Theme.typeXs
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space2

        Rectangle {
            width: alarmTxt.implicitWidth + 24
            height: 28
            radius: 8
            color: (clock && clock.masterAlarm) ? Theme.masterAlarm : Theme.bg2
            border.color: (clock && clock.masterAlarm) ? Theme.alarm : Theme.stroke
            Text {
                id: alarmTxt
                anchors.centerIn: parent
                text: (clock && clock.masterAlarm) ? "MASTER ALARM" : "Alarms clear"
                color: Theme.textPrimary
                font.pixelSize: Theme.typeXs
                font.weight: Font.Bold
            }
            // shape cue for alarm (diamond already used on KPI; here use triangle via text)
            Text {
                visible: clock && clock.masterAlarm
                anchors.left: parent.left
                anchors.leftMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                text: "!"
                color: Theme.textPrimary
                font.weight: Font.Black
            }
        }

        Button {
            text: "A−"
            onClicked: Theme.bumpFont(-5)
            background: Rectangle { radius: Theme.radiusCtl; color: Theme.bg2; border.color: Theme.stroke }
            contentItem: Text {
                text: parent.text; color: Theme.textPrimary; font.pixelSize: Theme.typeSm
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }
        Button {
            text: "A+"
            onClicked: Theme.bumpFont(5)
            background: Rectangle { radius: Theme.radiusCtl; color: Theme.bg2; border.color: Theme.stroke }
            contentItem: Text {
                text: parent.text; color: Theme.textPrimary; font.pixelSize: Theme.typeSm
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }
        Button {
            text: Theme.dark ? "Light" : "Dark"
            onClicked: Theme.toggleTheme()
            background: Rectangle { radius: Theme.radiusCtl; color: Theme.bg2; border.color: Theme.stroke }
            contentItem: Text {
                text: parent.text; color: Theme.textPrimary; font.pixelSize: Theme.typeSm
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
