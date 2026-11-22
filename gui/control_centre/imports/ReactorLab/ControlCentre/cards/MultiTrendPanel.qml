import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

Rectangle {
    id: root
    property var engine: null
    property int selected: 0 // 0 P% 1 rho 2 Tf 3 Tc 4 X
    radius: Theme.radiusCard
    color: Theme.bgCard
    border.color: Theme.stroke

    readonly property var labels: ["Power %", "ρ pcm", "Tf K", "Tc K", "Xe"]

    function series() {
        if (!engine) return []
        if (selected === 1) return engine.trendRho
        if (selected === 2) return engine.trendTf
        if (selected === 3) return engine.trendTc
        if (selected === 4) return engine.trendX
        // power fraction %
        var p = engine.trendP
        var out = []
        for (var i = 0; i < p.length; ++i) out.push(p[i] * 100.0)
        return out
    }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space3
        spacing: Theme.space2

        Row {
            width: parent.width
            spacing: Theme.space2
            Text {
                text: "Live trends"
                color: Theme.textPrimary
                font.pixelSize: Theme.typeMd
                font.weight: Font.DemiBold
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: "shared time · plant ODE"
                color: Theme.textMuted
                font.pixelSize: Theme.typeXs
                anchors.verticalCenter: parent.verticalCenter
            }
            Item { width: 12; height: 1 }
            Repeater {
                model: root.labels
                delegate: Button {
                    text: modelData
                    onClicked: root.selected = index
                    background: Rectangle {
                        radius: Theme.radiusCtl
                        color: root.selected === index ? Theme.accentDim : Theme.bg2
                        border.color: root.selected === index ? Theme.accent : Theme.stroke
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

        Canvas {
            id: chart
            width: parent.width
            height: parent.height - 40
            antialiasing: true

            Connections {
                target: root.engine
                function onTrendsChanged() { chart.requestPaint() }
                function onSnapshotChanged() { chart.requestPaint() }
            }
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            onVisibleChanged: requestPaint()

            Timer {
                interval: 100
                running: true
                repeat: true
                onTriggered: chart.requestPaint()
            }

            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()
                ctx.fillStyle = Theme.bg1
                ctx.fillRect(0, 0, width, height)

                var ys = root.series()
                var xs = root.engine ? root.engine.trendT : []
                if (!ys || ys.length < 2) {
                    ctx.fillStyle = Theme.textMuted
                    ctx.font = Theme.typeSm + "px sans-serif"
                    ctx.fillText("Press Play — live plant trajectory streams here", 16, height / 2)
                    return
                }

                var min = ys[0], max = ys[0]
                for (var i = 1; i < ys.length; ++i) {
                    min = Math.min(min, ys[i]); max = Math.max(max, ys[i])
                }
                if (max === min) { max += 1; min -= 1 }

                // downsample for paint
                var step = Math.max(1, Math.floor(ys.length / 800))

                ctx.strokeStyle = Theme.stroke
                ctx.lineWidth = 1
                for (var g = 0; g < 5; ++g) {
                    var gy = height * g / 4
                    ctx.beginPath(); ctx.moveTo(0, gy); ctx.lineTo(width, gy); ctx.stroke()
                }

                ctx.beginPath()
                var n = 0
                for (i = 0; i < ys.length; i += step) {
                    var x = (i / (ys.length - 1)) * (width - 8) + 4
                    var y = height - (ys[i] - min) / (max - min) * (height - 20) - 10
                    if (n === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                    ++n
                }
                ctx.strokeStyle = Theme.accent
                ctx.lineWidth = 2
                ctx.stroke()

                ctx.fillStyle = Theme.textMuted
                ctx.font = Theme.typeXs + "px monospace"
                ctx.fillText(max.toFixed(2), 8, 14)
                ctx.fillText(min.toFixed(2), 8, height - 6)
                if (xs.length)
                    ctx.fillText("t=" + Number(xs[xs.length - 1]).toFixed(1) + " s", width - 100, 14)
            }
        }
    }
}
