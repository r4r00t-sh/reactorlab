import QtQuick
import ReactorLab.Proto

Rectangle {
    id: root
    property int currentIndex: 0
    signal navigated(int index)

    width: Theme.railWidth
    color: Theme.bg1
    border.color: Theme.stroke

    readonly property var items: [
        { icon: "Ov", label: "Overview" },
        { icon: "Cr", label: "Core" },
        { icon: "Tr", label: "Trends" },
        { icon: "Rd", label: "Radiation" },
        { icon: "Th", label: "Thermal" },
        { icon: "Sc", label: "Scenarios" },
        { icon: "Al", label: "Alarms" },
        { icon: "Lg", label: "Logs" },
        { icon: "St", label: "Settings" }
    ]

    Column {
        anchors.fill: parent
        anchors.topMargin: Theme.space2
        spacing: Theme.space1

        Repeater {
            model: root.items.length
            delegate: Item {
                width: root.width
                height: 56
                property int idx: index
                property var item: root.items[index]

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 48
                    height: 48
                    radius: Theme.radiusCtl
                    color: root.currentIndex === parent.idx ? Theme.accentDim : "transparent"
                    border.color: root.currentIndex === parent.idx ? Theme.accent : "transparent"

                    Column {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: item.icon
                            font.family: Theme.fontMono
                            font.pixelSize: 13
                            font.weight: Font.Bold
                            color: Theme.accent
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: item.label
                            font.family: Theme.fontUi
                            font.pixelSize: 8
                            color: Theme.textMuted
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.currentIndex = idx
                            root.navigated(idx)
                        }
                    }
                }
            }
        }
    }
}
