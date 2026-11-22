import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ReactorLab.Proto

ApplicationWindow {
    id: win
    width: 1440
    height: 900
    minimumWidth: 1280
    minimumHeight: 720
    visible: true
    title: "ReactorLab · Control Centre (UI-0 prototype)"
    color: Theme.bg0

    FakeEngine { id: engine }

    Rectangle {
        id: banner
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 28
        color: Theme.warn
        z: 10
        Text {
            anchors.centerIn: parent
            text: "UI-0 PROTOTYPE — FAKE DATA — not connected to libreactorlab · educational only"
            color: "#1a1200"
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }
    }

    TopBar {
        id: top
        anchors.top: banner.bottom
        anchors.left: rail.right
        anchors.right: parent.right
        engine: engine
    }

    NavRail {
        id: rail
        anchors.top: banner.bottom
        anchors.bottom: transport.top
        anchors.left: parent.left
        currentIndex: pages.currentIndex
        onNavigated: (i) => { pages.currentIndex = i }
    }

    TransportBar {
        id: transport
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        engine: engine
    }

    StackLayout {
        id: pages
        anchors.top: top.bottom
        anchors.bottom: transport.top
        anchors.left: rail.right
        anchors.right: parent.right
        anchors.margins: Theme.space3
        currentIndex: 0

        Item {
            Column {
                anchors.fill: parent
                spacing: Theme.space3

                Text {
                    text: "Overview"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.typeLg
                    font.weight: Font.DemiBold
                }

                Flow {
                    width: parent.width
                    spacing: Theme.space3

                    KpiCard {
                        title: "Thermal power"
                        valueText: engine.powerPct.toFixed(1)
                        unit: "% rated"
                        spark: engine.sparkPower
                        accent: Theme.accent
                        arrow: engine.powerPct >= 100 ? "↑" : "↓"
                        warn: engine.powerPct > 110
                        alarm: engine.powerPct > 118
                    }
                    KpiCard {
                        title: "Reactivity"
                        valueText: engine.rhoPcm.toFixed(0)
                        unit: "pcm"
                        spark: engine.sparkRho
                        accent: engine.rhoPcm >= 0 ? Theme.warn : Theme.accent
                        arrow: engine.rhoPcm >= 0 ? "↑" : "↓"
                    }
                    KpiCard {
                        title: "Reactor period"
                        valueText: engine.periodS.toFixed(1)
                        unit: "s"
                        spark: engine.sparkPower
                        accent: Theme.accent
                    }
                    KpiCard {
                        title: "k-effective"
                        valueText: engine.keff.toFixed(5)
                        unit: "—"
                        spark: engine.sparkRho
                        accent: Theme.safe
                    }
                    KpiCard {
                        title: "Fuel temperature"
                        valueText: engine.tfuel.toFixed(0)
                        unit: "K"
                        spark: engine.trendTf
                        accent: Theme.warn
                        warn: engine.tfuel > 1100
                        alarm: engine.tfuel > 1250
                    }
                    KpiCard {
                        title: "Coolant temperature"
                        valueText: engine.tcool.toFixed(0)
                        unit: "K"
                        spark: engine.trendTc
                        accent: Theme.accent
                    }
                    KpiCard {
                        title: "Coolant flow"
                        valueText: (engine.mdot / 1000).toFixed(1)
                        unit: "t/s"
                        spark: engine.sparkPower
                        accent: Theme.accent
                    }
                    KpiCard {
                        title: "Xe worth"
                        valueText: engine.xenonWorthPcm.toFixed(0)
                        unit: "pcm"
                        spark: engine.sparkRho
                        accent: Theme.textMuted
                    }
                }

                Row {
                    width: parent.width
                    height: Math.max(280, parent.height - 300)
                    spacing: Theme.space3

                    TrendPanel {
                        width: parent.width * 0.58
                        height: parent.height
                        engine: engine
                    }
                    HexPreview {
                        width: parent.width * 0.42 - Theme.space3
                        height: parent.height
                        rodZ: engine.rodZ
                    }
                }
            }
        }

        Rectangle {
            radius: Theme.radiusCard
            color: Theme.bgCard
            border.color: Theme.stroke
            Column {
                anchors.centerIn: parent
                spacing: Theme.space2
                Text {
                    text: "Core (UI-3)"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.typeXl
                    horizontalAlignment: Text.AlignHCenter
                }
                Text {
                    width: 480
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    text: "Interactive hex map, rod elevation, and scram animation ship in UI-3 after the multi-rod engine decision."
                    color: Theme.textMuted
                }
                Slider {
                    width: 320
                    from: 0; to: 1; value: engine.rodZ
                    onMoved: engine.rodZ = value
                }
                Text {
                    text: "Demo rod insertion z = " + engine.rodZ.toFixed(2)
                    color: Theme.accent
                    font.family: Theme.fontMono
                }
            }
        }

        TrendPanel { engine: engine }

        Rectangle {
            radius: Theme.radiusCard
            color: Theme.bgCard
            border.color: Theme.alarm
            Column {
                anchors.centerIn: parent
                spacing: Theme.space3
                width: parent.width * 0.7
                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    text: "Radiation view gated"
                    color: Theme.alarm
                    font.pixelSize: Theme.typeXl
                    font.weight: Font.Bold
                }
                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    text: "No radiation / shielding model exists in libreactorlab. UI-6 will not invent dose maps. " +
                          "Approve engine addition C in docs/ui/000-audit-and-ui0-plan.md first. " +
                          "Future UI will be labeled educational estimates — not for dose assessment."
                    color: Theme.textPrimary
                    font.pixelSize: Theme.typeMd
                }
            }
        }

        Repeater {
            model: ["Thermal (UI-7)", "Scenarios (UI-7)", "Alarms (UI-7)", "Logs (UI-7)", "Settings (UI-1)"]
            delegate: Rectangle {
                required property string modelData
                radius: Theme.radiusCard
                color: Theme.bgCard
                border.color: Theme.stroke
                Text {
                    anchors.centerIn: parent
                    text: parent.modelData
                    color: Theme.textMuted
                    font.pixelSize: Theme.typeLg
                }
            }
        }
    }
}
