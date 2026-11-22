import QtQuick
import QtQuick.Controls
import ReactorLab.Proto

Rectangle {
    id: root
    property var engine: null
    radius: Theme.radiusCard
    color: Theme.bgCard
    border.color: Theme.stroke

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space3
        spacing: Theme.space2

        Row {
            width: parent.width
            spacing: Theme.space2
            Text {
                text: "Trends · demo data"
                color: Theme.textPrimary
                font.family: Theme.fontUi
                font.pixelSize: Theme.typeMd
                font.weight: Font.DemiBold
                anchors.verticalCenter: parent.verticalCenter
            }
            Item { width: parent.width - paramRow.width - 180; height: 1 }
            Row {
                id: paramRow
                spacing: Theme.space1
                Repeater {
                    model: ["Power", "ρ", "Tf", "Tc"]
                    delegate: Button {
                        text: modelData
                        flat: true
                        checkable: true
                        checked: engine && engine.selectedParam === index
                        onClicked: if (engine) engine.selectedParam = index
                        background: Rectangle {
                            radius: Theme.radiusCtl
                            color: parent.checked ? Theme.accentDim : Theme.bg2
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

        Canvas {
            id: chart
            width: parent.width
            height: parent.height - 48
            antialiasing: true

            property var xs: engine ? engine.trendT : []
            property var ys: {
                if (!engine) return []
                if (engine.selectedParam === 1) return engine.trendRho
                if (engine.selectedParam === 2) return engine.trendTf
                if (engine.selectedParam === 3) return engine.trendTc
                return engine.trendP
            }

            onXsChanged: requestPaint()
            onYsChanged: requestPaint()
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()

            Connections {
                target: engine
                function onTrendPChanged() { chart.requestPaint() }
                function onSelectedParamChanged() { chart.requestPaint() }
            }

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
                var ys = chart.ys
                var xs = chart.xs
                if (!ys || ys.length < 2) {
                    ctx.fillStyle = Theme.textMuted
                    ctx.font = Theme.typeSm + "px sans-serif"
                    ctx.fillText("Press Play to stream demo signals…", 16, height / 2)
                    return
                }
                var min = ys[0], max = ys[0]
                for (var i = 1; i < ys.length; ++i) {
                    min = Math.min(min, ys[i]); max = Math.max(max, ys[i])
                }
                if (max === min) { max += 1; min -= 1 }
                // grid
                ctx.strokeStyle = Theme.stroke
                ctx.lineWidth = 1
                for (var g = 0; g < 5; ++g) {
                    var gy = height * g / 4
                    ctx.beginPath(); ctx.moveTo(0, gy); ctx.lineTo(width, gy); ctx.stroke()
                }
                ctx.beginPath()
                for (i = 0; i < ys.length; ++i) {
                    var x = i / (ys.length - 1) * (width - 8) + 4
                    var y = height - (ys[i] - min) / (max - min) * (height - 16) - 8
                    if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                }
                ctx.strokeStyle = Theme.accent
                ctx.lineWidth = 2
                ctx.stroke()
                ctx.fillStyle = Theme.textMuted
                ctx.font = Theme.typeXs + "px monospace"
                ctx.fillText(max.toFixed(1), 8, 14)
                ctx.fillText(min.toFixed(1), 8, height - 6)
                if (xs.length)
                    ctx.fillText("t=" + xs[xs.length - 1].toFixed(1) + " s", width - 90, 14)
            }
        }
    }
}
