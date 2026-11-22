import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

// Synced bank elevators — every bank writes Engine.rodZ (educational until multi-bank API).
Rectangle {
    id: root
    property var engine: null

    radius: Theme.radiusCard
    color: Theme.bgCard
    border.color: Theme.stroke
    clip: true

    readonly property real rodZ: {
        if (!engine) return 0
        var z = Number(engine.rodZ)
        return isFinite(z) ? Math.max(0, Math.min(1, z)) : 0
    }
    readonly property real rhoPcm: {
        if (!engine) return 0
        var r = Number(engine.rhoPcm)
        return isFinite(r) ? r : 0
    }
    readonly property bool scrammed: engine && engine.simState === "Scram"

    readonly property var banks: [
        { id: "A", label: "Bank A", role: "regulating" },
        { id: "B", label: "Bank B", role: "regulating" },
        { id: "C", label: "Bank C", role: "regulating" },
        { id: "D", label: "Bank D", role: "regulating" },
        { id: "SA", label: "Safety", role: "scram" }
    ]

    function setZ(z) {
        if (!engine || scrammed) return
        engine.rodZ = Math.max(0, Math.min(1, z))
    }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space3
        spacing: Theme.space2

        Row {
            width: parent.width
            spacing: Theme.space2
            Text {
                text: "Rod banks"
                color: Theme.textPrimary
                font.pixelSize: Theme.typeMd
                font.weight: Font.DemiBold
            }
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: syncLbl.implicitWidth + 12
                height: 20
                radius: Theme.radiusPill
                color: Theme.warn
                Text {
                    id: syncLbl
                    anchors.centerIn: parent
                    text: "SYNCED"
                    color: "#1a1200"
                    font.pixelSize: Theme.typeXs
                    font.weight: Font.DemiBold
                }
            }
        }

        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            text: "All banks mirror plant rod_z. Independent bank worths are not modeled yet."
            color: Theme.textMuted
            font.pixelSize: Theme.typeXs
        }

        // Elevator strip
        Row {
            id: elevators
            width: parent.width
            height: Math.max(160, parent.height - 210)
            spacing: Theme.space2

            Repeater {
                model: root.banks
                delegate: Item {
                    id: bankCol
                    required property var modelData
                    width: (elevators.width - (root.banks.length - 1) * Theme.space2) / root.banks.length
                    height: elevators.height
                    readonly property bool isSafety: modelData.id === "SA"
                    readonly property color rodColor: isSafety ? Theme.alarm : Theme.warn

                    Column {
                        anchors.fill: parent
                        spacing: 4

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: bankCol.modelData.id
                            color: bankCol.isSafety ? Theme.alarm : Theme.textPrimary
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.typeSm
                            font.weight: Font.DemiBold
                        }

                        Item {
                            width: parent.width
                            height: parent.height - 36

                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: Math.min(28, parent.width - 8)
                                height: parent.height
                                radius: 8
                                color: Theme.bg2
                                border.color: Theme.stroke

                                Rectangle {
                                    anchors.top: parent.top
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    width: parent.width
                                    height: parent.height * root.rodZ
                                    radius: 8
                                    color: bankCol.rodColor
                                    Behavior on height {
                                        NumberAnimation { duration: Theme.durMed; easing.type: Easing.OutCubic }
                                    }
                                }

                                Rectangle {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    y: Math.max(0, parent.height * root.rodZ - 6)
                                    width: parent.width + 6
                                    height: 12
                                    radius: 4
                                    color: Theme.strokeStrong
                                    Behavior on y {
                                        NumberAnimation { duration: Theme.durMed; easing.type: Easing.OutCubic }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                enabled: !root.scrammed
                                cursorShape: Qt.SizeVerCursor
                                onPositionChanged: function (mouse) {
                                    if (!pressed) return
                                    root.setZ(mouse.y / height)
                                }
                                onClicked: function (mouse) {
                                    root.setZ(mouse.y / height)
                                }
                            }
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: (root.rodZ * 100).toFixed(0) + "%"
                            color: Theme.textMuted
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.typeXs
                        }
                    }
                }
            }
        }

        Text {
            text: "Master insertion"
            color: Theme.textMuted
            font.pixelSize: Theme.typeSm
        }
        Slider {
            id: master
            width: parent.width
            from: 0; to: 1
            value: root.rodZ
            enabled: !root.scrammed
            onMoved: root.setZ(value)
        }

        Row {
            width: parent.width
            spacing: Theme.space3
            Text {
                text: "ρ ≈ " + root.rhoPcm.toFixed(0) + " pcm"
                color: root.rhoPcm >= 0 ? Theme.warn : Theme.accent
                font.family: Theme.fontMono
                font.pixelSize: Theme.typeLg
                anchors.verticalCenter: parent.verticalCenter
            }
            Item { width: 12; height: 1 }
            Button {
                text: "SCRAM"
                enabled: !root.scrammed
                onClicked: if (root.engine) root.engine.scram()
                background: Rectangle {
                    radius: Theme.radiusCtl
                    color: root.scrammed ? Theme.bg2 : Theme.alarm
                    border.color: Theme.stroke
                }
                contentItem: Text {
                    text: parent.text
                    color: "#fff"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.typeSm
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Button {
                text: "Reset rods"
                enabled: !root.scrammed
                onClicked: root.setZ(0)
                background: Rectangle {
                    radius: Theme.radiusCtl
                    color: Theme.bg2
                    border.color: Theme.stroke
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    font.pixelSize: Theme.typeSm
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
