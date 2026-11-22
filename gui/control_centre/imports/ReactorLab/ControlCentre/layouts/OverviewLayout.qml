import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

Item {
    id: root
    property var clock: null

    function n(v, fallback) {
        var x = Number(v)
        return isFinite(x) ? x : fallback
    }
    function fmt(v, digits, fallback) {
        if (!root.clock) return fallback
        return root.n(v, 0).toFixed(digits)
    }

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
                valueText: root.fmt(root.clock ? root.clock.powerMW : 0, 0, "—")
                unit: "MWₜₕ"
                spark: root.clock ? root.clock.sparkPower : []
                accent: Theme.accent
                arrow: root.n(root.clock ? root.clock.powerPct : 0, 0) >= 100 ? "↑" : "↓"
                warn: root.n(root.clock ? root.clock.powerPct : 0, 0) > 110
                alarm: root.n(root.clock ? root.clock.powerPct : 0, 0) > 118
            }
            KpiCard {
                title: "Power fraction"
                valueText: root.fmt(root.clock ? root.clock.powerPct : 0, 1, "—")
                unit: "%"
                spark: root.clock ? root.clock.sparkPower : []
                accent: Theme.accent
            }
            KpiCard {
                title: "Reactivity"
                valueText: root.fmt(root.clock ? root.clock.rhoPcm : 0, 0, "—")
                unit: "pcm"
                spark: root.clock ? root.clock.sparkRho : []
                accent: root.n(root.clock ? root.clock.rhoPcm : 0, 0) >= 0 ? Theme.warn : Theme.accent
            }
            KpiCard {
                title: "Period"
                valueText: root.fmt(root.clock ? root.clock.periodS : 0, 1, "—")
                unit: "s"
                spark: root.clock ? root.clock.sparkPower : []
                accent: Theme.accent
            }
            KpiCard {
                title: "k-effective"
                valueText: root.fmt(root.clock ? root.clock.keff : 1, 5, "—")
                unit: "—"
                spark: root.clock ? root.clock.sparkRho : []
                accent: Theme.safe
            }
            KpiCard {
                title: "Fuel T"
                valueText: root.fmt(root.clock ? root.clock.tfuel : 0, 0, "—")
                unit: "K"
                spark: root.clock ? root.clock.sparkPower : []
                accent: Theme.warn
                warn: root.n(root.clock ? root.clock.tfuel : 0, 0) > 1100
            }
            KpiCard {
                title: "Coolant T"
                valueText: root.fmt(root.clock ? root.clock.tcool : 0, 0, "—")
                unit: "K"
                spark: root.clock ? root.clock.sparkPower : []
                accent: Theme.accent
            }
            KpiCard {
                title: "Xe worth"
                valueText: root.fmt(root.clock ? root.clock.xenonWorthPcm : 0, 0, "—")
                unit: "pcm"
                spark: root.clock ? root.clock.sparkRho : []
                accent: Theme.textMuted
            }
        }

        SplitView {
            width: parent.width
            height: Math.max(260, root.height - 280)
            orientation: Qt.Horizontal
            handle: Rectangle {
                implicitWidth: 6
                color: Theme.stroke
            }

            MultiTrendPanel {
                SplitView.preferredWidth: parent.width * 0.58
                SplitView.minimumWidth: 320
                engine: root.clock
            }

            HexCoreMap {
                SplitView.minimumWidth: 260
                SplitView.preferredWidth: parent.width * 0.42
                compact: true
                engine: root.clock
            }
        }
    }
}
