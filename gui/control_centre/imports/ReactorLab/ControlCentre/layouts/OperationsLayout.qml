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
            text: "Operations"
            color: Theme.textPrimary
            font.pixelSize: Theme.typeLg
            font.weight: Font.DemiBold
        }

        SplitView {
            width: parent.width
            height: parent.height - 40
            orientation: Qt.Horizontal
            handle: Rectangle { implicitWidth: 6; color: Theme.stroke }

            HexCoreMap {
                SplitView.preferredWidth: parent.width * 0.42
                SplitView.minimumWidth: 280
                engine: root.clock
            }

            RodBankPanel {
                SplitView.preferredWidth: parent.width * 0.32
                SplitView.minimumWidth: 240
                engine: root.clock
            }

            PlaceholderCard {
                SplitView.minimumWidth: 200
                title: "Process mimic"
                subtitle: "UI-7"
                body: "Core → loop → SG schematic with live flow/T/P ships in UI-7."
            }
        }
    }
}
