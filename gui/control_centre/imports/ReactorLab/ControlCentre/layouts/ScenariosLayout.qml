import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import ReactorLab.ControlCentre

Item {
    id: root
    property var clock: null

    Column {
        anchors.fill: parent
        spacing: Theme.space3

        Text {
            text: "Scenarios"
            color: Theme.textPrimary
            font.pixelSize: Theme.typeLg
            font.weight: Font.DemiBold
        }
        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            text: "Plant drivers only in this shell. Selecting a scenario resets state and sets educational time steps."
            color: Theme.textMuted
            font.pixelSize: Theme.typeXs
        }

        Text {
            text: "Active: " + (root.clock ? root.clock.scenarioName : "—")
            color: Theme.accent
            font.family: Theme.fontMono
            font.pixelSize: Theme.typeMd
        }

        Flow {
            width: parent.width
            spacing: Theme.space2
            Repeater {
                model: root.clock ? root.clock.scenarioIds.length : 0
                delegate: Button {
                    text: root.clock.scenarioLabels[index]
                    onClicked: root.clock.setScenario(root.clock.scenarioIds[index])
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
                        leftPadding: 10; rightPadding: 10
                    }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: Theme.stroke
        }

        Text {
            text: "Export trends (CSV)"
            color: Theme.textPrimary
            font.pixelSize: Theme.typeMd
            font.weight: Font.DemiBold
        }

        Row {
            spacing: Theme.space2
            Button {
                text: "Export CSV…"
                onClicked: saveDlg.open()
                background: Rectangle {
                    radius: Theme.radiusCtl
                    color: Theme.accentDim
                    border.color: Theme.accent
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    font.pixelSize: Theme.typeSm
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 12; rightPadding: 12
                }
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: root.clock && root.clock.exportStatus ? root.clock.exportStatus : "—"
                color: Theme.textMuted
                font.pixelSize: Theme.typeXs
                font.family: Theme.fontMono
            }
        }

        FileDialog {
            id: saveDlg
            fileMode: FileDialog.SaveFile
            nameFilters: ["CSV files (*.csv)"]
            defaultSuffix: "csv"
            title: "Export ReactorLab trends"
            onAccepted: {
                var path = selectedFile.toString()
                if (path.indexOf("file://") === 0)
                    path = path.substring(7)
                if (root.clock) root.clock.exportCsv(path)
            }
        }
    }
}
