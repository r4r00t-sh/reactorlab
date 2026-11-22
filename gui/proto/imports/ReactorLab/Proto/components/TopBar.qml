import QtQuick
import QtQuick.Controls
import ReactorLab.Proto

Rectangle {
    id: root
    property var engine: null
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
                text: engine ? engine.reactorName : "ReactorLab"
                color: Theme.textPrimary
                font.family: Theme.fontUi
                font.pixelSize: Theme.typeMd
                font.weight: Font.DemiBold
            }
            Text {
                text: "Educational control-centre prototype · not safety-grade"
                color: Theme.textMuted
                font.pixelSize: Theme.typeXs
            }
        }

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: stateLbl.implicitWidth + 20
            height: 28
            radius: 14
            color: {
                if (!engine) return Theme.bg2
                if (engine.simState === "Scram") return Theme.alarm
                if (engine.running) return Theme.safe
                return Theme.bg2
            }
            Text {
                id: stateLbl
                anchors.centerIn: parent
                text: engine ? engine.simState : "—"
                color: Theme.textPrimary
                font.pixelSize: Theme.typeSm
                font.weight: Font.Medium
            }
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: engine ? ("t = " + engine.simTime.toFixed(1) + " s   ×" + engine.speed.toFixed(1))
                         : ""
            color: Theme.textPrimary
            font.family: Theme.fontMono
            font.pixelSize: Theme.typeSm
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: engine ? engine.scenarioName : ""
            color: Theme.textMuted
            font.pixelSize: Theme.typeSm
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
            color: (engine && engine.masterAlarm) ? Theme.masterAlarm : Theme.bg2
            border.color: (engine && engine.masterAlarm) ? Theme.alarm : Theme.stroke
            Text {
                id: alarmTxt
                anchors.centerIn: parent
                text: (engine && engine.masterAlarm) ? "MASTER ALARM" : "Alarms clear"
                color: Theme.textPrimary
                font.pixelSize: Theme.typeXs
                font.weight: Font.Bold
            }
        }

        Button {
            text: Theme.dark ? "Light" : "Dark"
            onClicked: Theme.toggleTheme()
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
}
