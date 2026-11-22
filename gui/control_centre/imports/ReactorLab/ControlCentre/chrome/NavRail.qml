import QtQuick
import ReactorLab.ControlCentre

Rectangle {
    id: root
    property int currentIndex: LayoutStore.navIndex
    signal navigated(int index)

    width: Theme.railWidth
    color: Theme.bg1
    border.color: Theme.stroke

    readonly property var items: [
        { id: "ov", label: "Overview" },
        { id: "op", label: "Ops" },
        { id: "an", label: "Analysis" },
        { id: "rd", label: "Radiation" },
        { id: "th", label: "Thermal" },
        { id: "sc", label: "Scenarios" },
        { id: "al", label: "Alarms" },
        { id: "lg", label: "Logs" },
        { id: "st", label: "Settings" }
    ]

    Column {
        anchors.fill: parent
        anchors.topMargin: Theme.space2
        spacing: Theme.space1

        Repeater {
            model: root.items.length
            delegate: Item {
                width: root.width
                height: 54
                property int idx: index
                property var item: root.items[index]

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 52
                    height: 48
                    radius: Theme.radiusCtl
                    color: root.currentIndex === idx ? Theme.accentDim : "transparent"
                    border.color: root.currentIndex === idx ? Theme.accent : "transparent"

                    Behavior on color { ColorAnimation { duration: Theme.durFast } }

                    Column {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: item.id.toUpperCase()
                            color: Theme.accent
                            font.family: Theme.fontMono
                            font.pixelSize: 11
                            font.weight: Font.Bold
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: item.label
                            color: Theme.textMuted
                            font.family: Theme.fontUi
                            font.pixelSize: 9
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            LayoutStore.setNav(idx)
                            root.navigated(idx)
                        }
                    }
                }
            }
        }
    }
}
