import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

Rectangle {
    id: root
    property var engine: null
    property int mode: 0 // 0 power 1 flux 2 tfuel 3 tcool axial

    radius: Theme.radiusCard
    color: Theme.bgCard
    border.color: Theme.stroke
    clip: true

    readonly property var modeLabels: ["Power rel", "Flux rel", "Tfuel K", "Tc axial K"]

    function series() {
        if (!engine) return []
        if (mode === 1) return engine.mapFlux
        if (mode === 2) return engine.mapTfuel
        if (mode === 3) return engine.mapTcoolAxial
        return engine.mapPower
    }

    function nx() { return engine ? engine.mapNx : 0 }
    function ny() { return engine ? engine.mapNy : 0 }
    function isAxial() { return mode === 3 }

    // Viridis-ish stops (colorblind-safer than rainbow)
    readonly property var viridis: [
        "#440154", "#482878", "#3e4a89", "#31688e", "#26828e",
        "#1f9e89", "#35b779", "#6ece58", "#b5de2b", "#fde725"
    ]

    function lerpColor(c0, c1, t) {
        return Qt.rgba(
            c0.r + (c1.r - c0.r) * t,
            c0.g + (c1.g - c0.g) * t,
            c0.b + (c1.b - c0.b) * t,
            1)
    }

    function sampleViridis(t) {
        t = Math.max(0, Math.min(1, t))
        var n = viridis.length - 1
        var x = t * n
        var i = Math.floor(x)
        if (i >= n) return viridis[n]
        var f = x - i
        return lerpColor(Qt.color(viridis[i]), Qt.color(viridis[i + 1]), f)
    }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space3
        spacing: Theme.space2

        Row {
            width: parent.width
            spacing: Theme.space2
            Text {
                text: "Heatmaps"
                color: Theme.textPrimary
                font.pixelSize: Theme.typeMd
                font.weight: Font.DemiBold
                anchors.verticalCenter: parent.verticalCenter
            }
            Repeater {
                model: root.modeLabels
                delegate: Button {
                    text: modelData
                    onClicked: root.mode = index
                    background: Rectangle {
                        radius: Theme.radiusCtl
                        color: root.mode === index ? Theme.accentDim : Theme.bg2
                        border.color: root.mode === index ? Theme.accent : Theme.stroke
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.textPrimary
                        font.pixelSize: Theme.typeXs
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            text: engine && engine.mapNote ? engine.mapNote : "Waiting for map form factor…"
            color: Theme.warn
            font.pixelSize: Theme.typeXs
        }

        Item {
            width: parent.width
            height: parent.height - 72

            Canvas {
                id: canvas
                anchors.fill: parent
                anchors.rightMargin: 56
                antialiasing: false

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    ctx.fillStyle = Theme.bg1
                    ctx.fillRect(0, 0, width, height)

                    var vals = root.series()
                    if (!vals || vals.length < 2) {
                        ctx.fillStyle = Theme.textMuted
                        ctx.font = Theme.typeSm + "px sans-serif"
                        ctx.fillText("Maps appear after engine start (TWIGL form).", 12, height / 2)
                        return
                    }

                    var min = vals[0], max = vals[0]
                    for (var i = 1; i < vals.length; ++i) {
                        min = Math.min(min, vals[i])
                        max = Math.max(max, vals[i])
                    }
                    if (max === min) { max += 1e-9; min -= 1e-9 }

                    if (root.isAxial()) {
                        var n = vals.length
                        var bw = width / n
                        for (i = 0; i < n; ++i) {
                            var t = (vals[i] - min) / (max - min)
                            ctx.fillStyle = root.sampleViridis(t)
                            ctx.fillRect(i * bw, 0, Math.ceil(bw) + 1, height)
                        }
                        ctx.fillStyle = Theme.textMuted
                        ctx.font = Theme.typeXs + "px monospace"
                        ctx.fillText("inlet →", 8, height - 8)
                        ctx.fillText("← outlet", width - 64, height - 8)
                    } else {
                        var nx = root.nx()
                        var ny = root.ny()
                        if (nx < 1 || ny < 1 || vals.length < nx * ny) {
                            ctx.fillStyle = Theme.textMuted
                            ctx.fillText("Invalid map dims", 12, height / 2)
                            return
                        }
                        var cw = width / nx
                        var ch = height / ny
                        for (var r = 0; r < ny; ++r) {
                            for (var c = 0; c < nx; ++c) {
                                var v = vals[r * nx + c]
                                t = (v - min) / (max - min)
                                ctx.fillStyle = root.sampleViridis(t)
                                ctx.fillRect(c * cw, r * ch, Math.ceil(cw) + 1, Math.ceil(ch) + 1)
                            }
                        }
                    }

                    // stash for legend
                    canvas._min = min
                    canvas._max = max
                    legend.requestPaint()
                }

                property real _min: 0
                property real _max: 1

                Connections {
                    target: root.engine
                    function onSnapshotChanged() { canvas.requestPaint() }
                }
                Connections {
                    target: root
                    function onModeChanged() { canvas.requestPaint() }
                }
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
            }

            Canvas {
                id: legend
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 52
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    ctx.fillStyle = Theme.bgCard
                    ctx.fillRect(0, 0, width, height)
                    var h = height - 28
                    for (var y = 0; y < h; ++y) {
                        var t = 1 - y / Math.max(1, h - 1)
                        ctx.fillStyle = root.sampleViridis(t)
                        ctx.fillRect(8, 14 + y, 14, 1)
                    }
                    ctx.fillStyle = Theme.textMuted
                    ctx.font = Theme.typeXs + "px monospace"
                    ctx.fillText(Number(canvas._max).toFixed(2), 26, 22)
                    ctx.fillText(Number(canvas._min).toFixed(2), 26, height - 8)
                }
            }
        }
    }
}
