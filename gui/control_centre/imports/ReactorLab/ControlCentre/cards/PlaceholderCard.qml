import QtQuick
import ReactorLab.ControlCentre

Card {
    id: root
    property string body: ""

    Text {
        width: parent.width
        height: parent.height
        text: root.body
        color: Theme.textMuted
        font.pixelSize: Theme.typeSm
        wrapMode: Text.WordWrap
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
