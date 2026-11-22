import QtQuick
import ReactorLab.Proto

// UI-0 placeholder hex strip — NOT engine geometry.
Rectangle {
    id: root
    property real rodZ: 0.35
    radius: Theme.radiusCard
    color: Theme.bgCard
    border.color: Theme.stroke

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space3
        spacing: Theme.space2

        Text {
            text: "Core map preview (fake lattice)"
            color: Theme.textPrimary
            font.pixelSize: Theme.typeMd
            font.weight: Font.DemiBold
        }
        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            text: "Placeholder only. Real hex map needs approved multi-rod / lattice data. " +
                  "Depth shown uses the single demo rodZ."
            color: Theme.warn
            font.pixelSize: Theme.typeXs
        }

        Canvas {
            id: hex
            width: parent.width
            height: parent.height - 72
            antialiasing: true
            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()
                ctx.fillStyle = Theme.bg1
                ctx.fillRect(0, 0, width, height)
                var R = 16
                var dx = R * 1.75
                var dy = R * 1.55
                var rows = Math.floor(height / dy) - 1
                var cols = Math.floor(width / dx) - 1
                function drawHex(cx, cy, fill) {
                    ctx.beginPath()
                    for (var k = 0; k < 6; ++k) {
                        var a = Math.PI / 6 + k * Math.PI / 3
                        var x = cx + R * Math.cos(a)
                        var y = cy + R * Math.sin(a)
                        if (k === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                    }
                    ctx.closePath()
                    ctx.fillStyle = fill
                    ctx.fill()
                    ctx.strokeStyle = Theme.stroke
                    ctx.stroke()
                }
                for (var r = 0; r < rows; ++r) {
                    for (var c = 0; c < cols; ++c) {
                        var cx = 24 + c * dx + (r % 2 ? dx * 0.5 : 0)
                        var cy = 24 + r * dy
                        var kind = (c + r) % 7
                        var fill = Theme.bg2
                        if (kind === 0) fill = Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.35) // fuel-ish
                        if (kind === 1) {
                            // control — deepen with rodZ
                            var g = 0.25 + root.rodZ * 0.55
                            fill = Qt.rgba(0.95, 0.55, 0.1, g)
                        }
                        if (kind === 2) fill = Qt.rgba(Theme.alarm.r, Theme.alarm.g, Theme.alarm.b, 0.45) // safety
                        drawHex(cx, cy, fill)
                    }
                }
            }
            Connections {
                target: root
                function onRodZChanged() { hex.requestPaint() }
            }
            Timer { interval: 200; running: true; repeat: true; onTriggered: hex.requestPaint() }
        }
    }
}
