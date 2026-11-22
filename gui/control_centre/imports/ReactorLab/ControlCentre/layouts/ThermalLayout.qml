import QtQuick
import ReactorLab.ControlCentre

Item {
    id: root
    property var clock: null

    readonly property real powerPct: clock ? Number(clock.powerPct) : 100
    readonly property real tf: clock ? Number(clock.tfuel) : 900
    readonly property real tc: clock ? Number(clock.tcool) : 580
    readonly property real rod: clock ? Number(clock.rodZ) : 0
    readonly property real mdot: clock ? Number(clock.mdotProxy || clock.mdot) : 29000

    Column {
        anchors.fill: parent
        spacing: Theme.space3

        Text {
            text: "Thermal / process mimic"
            color: Theme.textPrimary
            font.pixelSize: Theme.typeLg
            font.weight: Font.DemiBold
        }
        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            text: "Educational loop schematic — not a nodal plant model. ṁ is a power-scaled proxy."
            color: Theme.warn
            font.pixelSize: Theme.typeXs
        }

        Rectangle {
            width: parent.width
            height: parent.height - 70
            radius: Theme.radiusCard
            color: Theme.bgCard
            border.color: Theme.stroke

            Canvas {
                id: mimic
                anchors.fill: parent
                anchors.margins: Theme.space3
                antialiasing: true

                Connections {
                    target: root.clock
                    function onSnapshotChanged() { mimic.requestPaint() }
                }
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    ctx.fillStyle = Theme.bg1
                    ctx.fillRect(0, 0, width, height)

                    var cx = width * 0.22
                    var cy = height * 0.5
                    var sgx = width * 0.72
                    var sgy = height * 0.5
                    var pumpx = width * 0.47
                    var pumpy = height * 0.78

                    // pipes
                    ctx.strokeStyle = Theme.accent
                    ctx.lineWidth = 8
                    ctx.lineCap = "round"
                    ctx.beginPath()
                    ctx.moveTo(cx + 70, cy - 20)
                    ctx.lineTo(sgx - 70, cy - 40)
                    ctx.stroke()
                    ctx.beginPath()
                    ctx.moveTo(sgx - 70, cy + 40)
                    ctx.lineTo(pumpx + 30, pumpy)
                    ctx.lineTo(cx + 70, cy + 30)
                    ctx.stroke()

                    // core
                    var coreR = 55
                    ctx.fillStyle = Qt.rgba(0.18, 0.55, 0.5, 0.35 + 0.4 * Math.min(1, root.powerPct / 120))
                    ctx.beginPath()
                    ctx.arc(cx, cy, coreR, 0, Math.PI * 2)
                    ctx.fill()
                    ctx.strokeStyle = Theme.strokeStrong
                    ctx.lineWidth = 2
                    ctx.stroke()
                    ctx.fillStyle = Theme.textPrimary
                    ctx.font = "bold " + Theme.typeMd + "px sans-serif"
                    ctx.textAlign = "center"
                    ctx.fillText("CORE", cx, cy - 8)
                    ctx.font = Theme.typeXs + "px monospace"
                    ctx.fillStyle = Theme.accent
                    ctx.fillText(root.powerPct.toFixed(0) + "% · " + root.tf.toFixed(0) + " K", cx, cy + 14)

                    // rods overlay
                    ctx.fillStyle = Theme.warn
                    ctx.fillRect(cx - 6, cy - coreR + 8, 12, (2 * coreR - 16) * root.rod)

                    // SG
                    ctx.fillStyle = Theme.bg2
                    roundRect(ctx, sgx - 55, sgy - 70, 110, 140, 16)
                    ctx.fill()
                    ctx.strokeStyle = Theme.stroke
                    ctx.stroke()
                    ctx.fillStyle = Theme.textPrimary
                    ctx.font = "bold " + Theme.typeMd + "px sans-serif"
                    ctx.fillText("SG", sgx, sgy - 10)
                    ctx.font = Theme.typeXs + "px monospace"
                    ctx.fillStyle = Theme.textMuted
                    ctx.fillText("Tc " + root.tc.toFixed(0) + " K", sgx, sgy + 14)

                    // pump
                    ctx.fillStyle = Theme.bg2
                    ctx.beginPath()
                    ctx.arc(pumpx, pumpy, 28, 0, Math.PI * 2)
                    ctx.fill()
                    ctx.strokeStyle = Theme.accent
                    ctx.stroke()
                    ctx.fillStyle = Theme.textPrimary
                    ctx.font = "bold " + Theme.typeSm + "px sans-serif"
                    ctx.fillText("PUMP", pumpx, pumpy + 4)
                    ctx.font = Theme.typeXs + "px monospace"
                    ctx.fillStyle = Theme.textMuted
                    ctx.fillText((root.mdot / 1000).toFixed(1) + " t/s*", pumpx, pumpy + 48)

                    ctx.fillStyle = Theme.textMuted
                    ctx.textAlign = "left"
                    ctx.font = Theme.typeXs + "px sans-serif"
                    ctx.fillText("* ṁ proxy · educational", 12, height - 8)
                }

                function roundRect(ctx, x, y, w, h, r) {
                    ctx.beginPath()
                    ctx.moveTo(x + r, y)
                    ctx.arcTo(x + w, y, x + w, y + h, r)
                    ctx.arcTo(x + w, y + h, x, y + h, r)
                    ctx.arcTo(x, y + h, x, y, r)
                    ctx.arcTo(x, y, x + w, y, r)
                    ctx.closePath()
                }
            }
        }
    }
}
