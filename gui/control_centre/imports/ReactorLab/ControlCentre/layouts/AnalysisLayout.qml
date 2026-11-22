import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

Item {
    id: root
    property var clock: null

    Column {
        anchors.fill: parent
        spacing: Theme.space3

        Text {
            text: "Analysis"
            color: Theme.textPrimary
            font.pixelSize: Theme.typeLg
            font.weight: Font.DemiBold
        }

        SplitView {
            width: parent.width
            height: parent.height - 40
            orientation: Qt.Vertical
            handle: Rectangle { implicitHeight: 6; color: Theme.stroke }

            MultiTrendPanel {
                SplitView.preferredHeight: parent.height * 0.45
                SplitView.minimumHeight: 180
                engine: root.clock
            }

            SplitView {
                orientation: Qt.Horizontal
                handle: Rectangle { implicitWidth: 6; color: Theme.stroke }
                HeatmapPanel {
                    SplitView.preferredWidth: parent.width * 0.62
                    SplitView.minimumWidth: 320
                    engine: root.clock
                }
                Core3DView {
                    SplitView.minimumWidth: 280
                    engine: root.clock
                }
            }
        }
    }
}
