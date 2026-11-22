import QtQuick
import ReactorLab.ControlCentre

Rectangle {
    id: root
    property string title: ""
    property string valueText: "—"
    property string unit: ""
    property string arrow: "→"
    property color accent: Theme.accent
    property var spark: []
    property bool alarm: false
    property bool warn: false

    implicitWidth: 200
    implicitHeight: 118
    width: implicitWidth
    height: implicitHeight
    radius: Theme.radiusCard
    color: Theme.bgCard
    border.width: alarm || warn ? 2 : 1
    border.color: alarm ? Theme.alarm : (warn ? Theme.warn : Theme.stroke)

    Behavior on border.color { ColorAnimation { duration: Theme.durFast } }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space3
        spacing: 2

        Row {
            width: parent.width
            Text {
                width: parent.width - 24
                text: root.title
                color: Theme.textMuted
                font.family: Theme.fontUi
                font.pixelSize: Theme.typeXs
                elide: Text.ElideRight
            }
            Text {
                text: root.arrow
                color: root.accent
                font.pixelSize: Theme.typeSm
            }
        }

        Row {
            spacing: Theme.space2
            Text {
                text: root.valueText
                color: Theme.textPrimary
                font.family: Theme.fontMono
                font.pixelSize: Theme.typeKpi
                font.weight: Font.Medium
            }
            Text {
                text: root.unit
                color: Theme.textMuted
                font.pixelSize: Theme.typeSm
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Sparkline {
            width: parent.width
            height: 30
            values: root.spark
            stroke: root.accent
        }
    }

    // alarm also uses shape, not color alone
    Rectangle {
        visible: root.alarm
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        width: 10
        height: 10
        rotation: 45
        color: Theme.alarm
    }
}
