import QtQuick
import ReactorLab.ControlCentre

// Educational EDU-1 hex lattice — UI metadata only; control depth = synced rod_z.
Rectangle {
    id: root
    property var engine: null
    property bool compact: false
    property int hoverIndex: -1

    radius: Theme.radiusCard
    color: Theme.bgCard
    border.color: Theme.stroke
    clip: true

    readonly property real rodZ: {
        if (!engine) return 0
        var z = Number(engine.rodZ)
        return isFinite(z) ? Math.max(0, Math.min(1, z)) : 0
    }
    readonly property real powerPct: {
        if (!engine) return 100
        var p = Number(engine.powerPct)
        return isFinite(p) ? p : 100
    }
    readonly property bool scrammed: engine && engine.simState === "Scram"

    // Fixed axial cells: {q,r, kind, bank}
    // kind: 0 fuel, 1 control, 2 safety, 3 reflector
    readonly property var cells: {
        var out = []
        function add(q, r, kind, bank) { out.push({ q: q, r: r, kind: kind, bank: bank || "" }) }

        add(0, 0, 0, "")

        // ring 1
        var r1 = [[1, 0], [0, 1], [-1, 1], [-1, 0], [0, -1], [1, -1]]
        for (var i = 0; i < r1.length; ++i)
            add(r1[i][0], r1[i][1], i % 2 === 0 ? 1 : 0, i % 2 === 0 ? "A" : "")

        // ring 2 — mix fuel + banks B/C
        var r2 = [
            [2, 0, 1, "B"], [1, 1, 0, ""], [0, 2, 1, "B"],
            [-1, 2, 0, ""], [-2, 2, 1, "C"], [-2, 1, 0, ""],
            [-2, 0, 1, "C"], [-1, -1, 0, ""], [0, -2, 1, "B"],
            [1, -2, 0, ""], [2, -2, 1, "C"], [2, -1, 0, ""]
        ]
        for (i = 0; i < r2.length; ++i)
            add(r2[i][0], r2[i][1], r2[i][2], r2[i][3])

        // ring 3 — reflector + D + SA
        var r3 = [
            [3, 0, 2, "SA"], [2, 1, 3, ""], [1, 2, 1, "D"], [0, 3, 2, "SA"],
            [-1, 3, 3, ""], [-2, 3, 1, "D"], [-3, 3, 2, "SA"], [-3, 2, 3, ""],
            [-3, 1, 1, "D"], [-3, 0, 2, "SA"], [-2, -1, 3, ""], [-1, -2, 1, "D"],
            [0, -3, 2, "SA"], [1, -3, 3, ""], [2, -3, 1, "D"], [3, -3, 2, "SA"],
            [3, -2, 3, ""], [3, -1, 3, ""]
        ]
        for (i = 0; i < r3.length; ++i)
            add(r3[i][0], r3[i][1], r3[i][2], r3[i][3])

        return out
    }

    function axialPixel(q, r, size) {
        return {
            x: size * (Math.sqrt(3) * q + Math.sqrt(3) / 2 * r),
            y: size * (1.5 * r)
        }
    }

    function fillFor(cell) {
        var z = root.rodZ
        if (cell.kind === 0) {
            var f = Math.max(0.15, Math.min(1.0, root.powerPct / 120.0))
            return Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.2 + 0.55 * f)
        }
        if (cell.kind === 1) {
            var a = 0.28 + 0.62 * z
            return Qt.rgba(0.92, 0.58, 0.12, a)
        }
        if (cell.kind === 2) {
            var sa = root.scrammed ? 0.85 : (0.35 + 0.5 * z)
            return Qt.rgba(Theme.alarm.r, Theme.alarm.g, Theme.alarm.b, sa)
        }
        return Qt.rgba(Theme.textMuted.r, Theme.textMuted.g, Theme.textMuted.b, 0.22)
    }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space3
        spacing: Theme.space2

        Row {
            width: parent.width
            spacing: Theme.space2
            Text {
                text: root.compact ? "Core map" : "Hex core map"
                color: Theme.textPrimary
                font.pixelSize: Theme.typeMd
                font.weight: Font.DemiBold
            }
            Text {
                text: "EDU-1 · synced rod_z"
                color: Theme.warn
                font.pixelSize: Theme.typeXs
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                visible: !root.compact
                text: {
                    if (root.hoverIndex < 0) return "hover a cell"
                    var cell = root.cells[root.hoverIndex]
                    if (cell.bank)
                        return "bank " + cell.bank + " · z=" + root.rodZ.toFixed(2)
                    return ["fuel", "control", "safety", "reflector"][cell.kind]
                           + " · (" + cell.q + "," + cell.r + ")"
                }
                color: Theme.textMuted
                font.family: Theme.fontMono
                font.pixelSize: Theme.typeXs
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Text {
            visible: !root.compact
            width: parent.width
            wrapMode: Text.WordWrap
            text: "Lattice is educational metadata. All control/safety banks mirror the single plant rod_z until multi-bank physics ships."
            color: Theme.textMuted
            font.pixelSize: Theme.typeXs
        }

        Item {
            id: mapHost
            width: parent.width
            height: parent.height - (root.compact ? 28 : 64)

            Canvas {
                id: canvas
                anchors.fill: parent
                antialiasing: true

                property real hexSize: Math.min(width, height) / 10.5

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    ctx.fillStyle = Theme.bg1
                    ctx.fillRect(0, 0, width, height)

                    var size = canvas.hexSize
                    var cx0 = width * 0.5
                    var cy0 = height * 0.5

                    function drawHex(cx, cy, fill, highlight, label) {
                        ctx.beginPath()
                        for (var k = 0; k < 6; ++k) {
                            var a = Math.PI / 6 + k * Math.PI / 3
                            var x = cx + size * Math.cos(a)
                            var y = cy + size * Math.sin(a)
                            if (k === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                        }
                        ctx.closePath()
                        ctx.fillStyle = fill
                        ctx.fill()
                        ctx.lineWidth = highlight ? 2.5 : 1
                        ctx.strokeStyle = highlight ? Theme.accent : Theme.stroke
                        ctx.stroke()
                        if (label) {
                            ctx.fillStyle = Theme.textPrimary
                            ctx.font = "bold " + Math.max(9, Math.floor(size * 0.45)) + "px sans-serif"
                            ctx.textAlign = "center"
                            ctx.textBaseline = "middle"
                            ctx.fillText(label, cx, cy)
                        }
                    }

                    for (var i = 0; i < root.cells.length; ++i) {
                        var cell = root.cells[i]
                        var p = root.axialPixel(cell.q, cell.r, size)
                        var label = (cell.kind === 1 || cell.kind === 2) ? cell.bank : ""
                        drawHex(cx0 + p.x, cy0 + p.y, root.fillFor(cell), i === root.hoverIndex, label)
                    }

                    // legend strip
                    if (!root.compact) {
                        var lx = 10, ly = height - 18
                        ctx.font = Theme.typeXs + "px sans-serif"
                        ctx.fillStyle = Theme.textMuted
                        ctx.textAlign = "left"
                        ctx.textBaseline = "middle"
                        ctx.fillStyle = Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.55)
                        ctx.fillRect(lx, ly - 5, 10, 10)
                        ctx.fillStyle = Theme.textMuted
                        ctx.fillText("fuel", lx + 14, ly)
                        ctx.fillStyle = Qt.rgba(0.92, 0.58, 0.12, 0.75)
                        ctx.fillRect(lx + 52, ly - 5, 10, 10)
                        ctx.fillStyle = Theme.textMuted
                        ctx.fillText("control", lx + 66, ly)
                        ctx.fillStyle = Qt.rgba(Theme.alarm.r, Theme.alarm.g, Theme.alarm.b, 0.75)
                        ctx.fillRect(lx + 128, ly - 5, 10, 10)
                        ctx.fillStyle = Theme.textMuted
                        ctx.fillText("safety", lx + 142, ly)
                        ctx.fillStyle = Qt.rgba(Theme.textMuted.r, Theme.textMuted.g, Theme.textMuted.b, 0.35)
                        ctx.fillRect(lx + 198, ly - 5, 10, 10)
                        ctx.fillStyle = Theme.textMuted
                        ctx.fillText("reflector", lx + 212, ly)
                    }
                }

                function hitIndex(mx, my) {
                    var size = canvas.hexSize
                    var cx0 = width * 0.5
                    var cy0 = height * 0.5
                    var best = -1
                    var bestD = size * size
                    for (var i = 0; i < root.cells.length; ++i) {
                        var cell = root.cells[i]
                        var p = root.axialPixel(cell.q, cell.r, size)
                        var dx = mx - (cx0 + p.x)
                        var dy = my - (cy0 + p.y)
                        var d = dx * dx + dy * dy
                        if (d < bestD && d < size * size * 0.85) {
                            bestD = d
                            best = i
                        }
                    }
                    return best
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onPositionChanged: function (mouse) {
                        root.hoverIndex = canvas.hitIndex(mouse.x, mouse.y)
                        canvas.requestPaint()
                    }
                    onExited: { root.hoverIndex = -1; canvas.requestPaint() }
                    onClicked: function (mouse) {
                        var i = canvas.hitIndex(mouse.x, mouse.y)
                        if (i < 0 || !root.engine) return
                        var cell = root.cells[i]
                        if (cell.kind === 1 || cell.kind === 2) {
                            // nudge insertion toward full-in on control/safety click
                            var z = root.rodZ
                            root.engine.rodZ = Math.min(1, z + 0.05)
                        }
                    }
                }
            }

            Connections {
                target: root.engine
                function onSnapshotChanged() { canvas.requestPaint() }
            }
            Connections {
                target: root
                function onRodZChanged() { canvas.requestPaint() }
                function onPowerPctChanged() { canvas.requestPaint() }
                function onScrammedChanged() { canvas.requestPaint() }
            }
            onWidthChanged: canvas.requestPaint()
            onHeightChanged: canvas.requestPaint()
        }
    }

    // Scram flash ring
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.width: root.scrammed ? 2 : 0
        border.color: Theme.alarm
        opacity: root.scrammed ? 0.9 : 0
        Behavior on opacity { NumberAnimation { duration: Theme.durMed } }
    }
}
