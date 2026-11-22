import QtQuick
import ReactorLab.ControlCentre

Flow {
    id: root
    property int columns: 4
    property int gap: Theme.space3
    spacing: gap
    width: parent ? parent.width : 800
}
