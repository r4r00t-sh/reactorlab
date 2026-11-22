import QtQuick
import ReactorLab.ControlCentre

Rectangle {
    id: root
    property string title: ""
    property string subtitle: ""
    default property alias content: body.data

    radius: Theme.radiusCard
    color: Theme.bgCard
    border.color: Theme.stroke
    clip: true

    Behavior on color { ColorAnimation { duration: Theme.durFast } }
    Behavior on border.color { ColorAnimation { duration: Theme.durFast } }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space3
        spacing: Theme.space2

        Row {
            width: parent.width
            spacing: Theme.space2
            Text {
                text: root.title
                color: Theme.textPrimary
                font.family: Theme.fontUi
                font.pixelSize: Theme.typeMd
                font.weight: Font.DemiBold
            }
            Text {
                text: root.subtitle
                color: Theme.textMuted
                font.pixelSize: Theme.typeXs
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Item {
            id: body
            width: parent.width
            height: parent.height - 28
        }
    }
}
