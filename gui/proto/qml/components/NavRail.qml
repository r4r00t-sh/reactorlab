import QtQuick
import QtQuick.Controls
import ReactorLab.Proto

Rectangle {
    id: root
    property int currentIndex: 0
    signal navigated(int index)

    width: Theme.railWidth
    color: Theme.bg1
    border.color: Theme.stroke

    readonly property var items: [
        { icon: "◉", label: "Overview" },
        { icon: "⬡", label: "Core" },
        { icon: "📈", label: "Trends" },
        { icon: "☢", label: "Radiation" },
        { icon: "🌡", label: "Thermal" },
        { icon: "▶", label: "Scenarios" },
        { icon: "⚠", label: "Alarms" },
        { icon: "☰", label: "Logs" },
        { icon: "⚙", label: "Settings" }
    ]

    Column {
        anchors.fill: parent
        anchors.topMargin: Theme.space2
        spacing: Theme.space1

        Repeater {
            model: root.items
            delegate: Item {
                width: root.width
                height: 56
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 48
                    height: 48
                    radius: Theme.radiusCtl
                    color: root.currentIndex === index ? Theme.accentDim : "transparent"
                    border.color: root.currentIndex === index ? Theme.accent : "transparent"

                    Column {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.icon
                            font.pixelSize: 16
                            color: Theme.textPrimary
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.label
                            font.family: Theme.fontUi
                            font.pixelSize: 8
                            color: Theme.textMuted
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.currentIndex = index
                            root.navigated(index)
                        }
                    }
                }
            }
        }
    }
}
