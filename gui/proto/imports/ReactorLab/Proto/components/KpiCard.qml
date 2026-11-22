import QtQuick
import QtQuick.Controls
import ReactorLab.Proto

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

    width: 200
    height: 112
    radius: Theme.radiusCard
    color: Theme.bgCard
    border.color: alarm ? Theme.alarm : (warn ? Theme.warn : Theme.stroke)
    border.width: alarm || warn ? 2 : 1

    Behavior on border.color { ColorAnimation { duration: Theme.durFast } }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space3
        spacing: Theme.space1

        Row {
            width: parent.width
            Text {
                text: root.title
                color: Theme.textMuted
                font.family: Theme.fontUi
                font.pixelSize: Theme.typeXs
                width: parent.width - 28
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
            baselineOffset: Theme.typeKpi
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
                font.family: Theme.fontUi
                font.pixelSize: Theme.typeSm
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Sparkline {
            width: parent.width
            height: 28
            values: root.spark
            stroke: root.accent
        }
    }
}
