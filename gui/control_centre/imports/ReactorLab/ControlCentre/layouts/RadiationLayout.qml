import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

Item {
    id: root
    property var clock: null

    function fmtAct(bq) {
        var a = Number(bq)
        if (!isFinite(a) || a <= 0) return "0 Bq"
        if (a >= 1e9) return (a / 1e9).toFixed(2) + " GBq"
        if (a >= 1e6) return (a / 1e6).toFixed(2) + " MBq"
        if (a >= 1e3) return (a / 1e3).toFixed(2) + " kBq"
        return a.toFixed(1) + " Bq"
    }

    Column {
        anchors.fill: parent
        spacing: Theme.space3

        Rectangle {
            width: parent.width
            height: bannerCol.implicitHeight + 20
            radius: Theme.radiusCard
            color: Theme.alarm
            Column {
                id: bannerCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: Theme.space3
                spacing: 4
                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: "EDUCATIONAL ESTIMATES — NOT FOR SAFETY OR DOSE ASSESSMENT"
                    color: "#fff"
                    font.pixelSize: Theme.typeMd
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                }
                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: root.clock && root.clock.radNote
                          ? root.clock.radNote
                          : "Point-source γ/n with slab shield; α/β activity only."
                    color: "#ffe8ea"
                    font.pixelSize: Theme.typeXs
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        Text {
            text: "Radiation"
            color: Theme.textPrimary
            font.pixelSize: Theme.typeLg
            font.weight: Font.DemiBold
        }

        SplitView {
            width: parent.width
            height: parent.height - 120
            orientation: Qt.Horizontal
            handle: Rectangle { implicitWidth: 6; color: Theme.stroke }

            // Inventory
            Rectangle {
                SplitView.preferredWidth: parent.width * 0.34
                SplitView.minimumWidth: 240
                radius: Theme.radiusCard
                color: Theme.bgCard
                border.color: Theme.stroke
                Column {
                    anchors.fill: parent
                    anchors.margins: Theme.space3
                    spacing: Theme.space2
                    Text {
                        text: "Inventory (proxy groups)"
                        color: Theme.textPrimary
                        font.pixelSize: Theme.typeMd
                        font.weight: Font.DemiBold
                    }
                    Repeater {
                        model: root.clock ? root.clock.radNames.length : 0
                        delegate: Row {
                            width: parent.width
                            spacing: Theme.space2
                            property string kind: root.clock.radKinds[index]
                            property real act: Number(root.clock.radActivityBq[index])
                            property real maxAct: {
                                var m = 1
                                if (!root.clock) return 1
                                for (var i = 0; i < root.clock.radActivityBq.length; ++i)
                                    m = Math.max(m, Number(root.clock.radActivityBq[i]))
                                return m
                            }
                            Text {
                                width: 110
                                text: root.clock.radNames[index]
                                color: Theme.textMuted
                                font.pixelSize: Theme.typeXs
                                elide: Text.ElideRight
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Rectangle {
                                width: Math.max(40, parent.width - 200)
                                height: 12
                                radius: 4
                                color: Theme.bg2
                                Rectangle {
                                    width: parent.width * (act / maxAct)
                                    height: parent.height
                                    radius: 4
                                    color: kind === "alpha" ? Theme.alarm
                                         : (kind === "beta" ? Theme.warn : Theme.accent)
                                }
                            }
                            Text {
                                text: root.fmtAct(act)
                                color: Theme.textPrimary
                                font.family: Theme.fontMono
                                font.pixelSize: Theme.typeXs
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                    Rectangle {
                        width: parent.width
                        height: 1
                        color: Theme.stroke
                    }
                    Text {
                        text: "α total  " + root.fmtAct(root.clock ? root.clock.radAlphaBq : 0)
                              + "  (activity only — cm-scale range)"
                        color: Theme.alarm
                        font.pixelSize: Theme.typeXs
                        width: parent.width
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        text: "β total  " + root.fmtAct(root.clock ? root.clock.radBetaBq : 0)
                              + "  (activity only — short range)"
                        color: Theme.warn
                        font.pixelSize: Theme.typeXs
                        width: parent.width
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        text: "γ total  " + root.fmtAct(root.clock ? root.clock.radGammaBq : 0)
                        color: Theme.accent
                        font.pixelSize: Theme.typeXs
                    }
                }
            }

            // Dose curve
            Rectangle {
                SplitView.minimumWidth: 320
                radius: Theme.radiusCard
                color: Theme.bgCard
                border.color: Theme.stroke
                Column {
                    anchors.fill: parent
                    anchors.margins: Theme.space3
                    spacing: Theme.space2

                    Text {
                        text: "γ / n dose vs distance"
                        color: Theme.textPrimary
                        font.pixelSize: Theme.typeMd
                        font.weight: Font.DemiBold
                    }

                    Row {
                        width: parent.width
                        spacing: Theme.space2
                        Text {
                            text: "Shield"
                            color: Theme.textMuted
                            font.pixelSize: Theme.typeSm
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Slider {
                            id: shield
                            width: Math.min(220, parent.width - 160)
                            from: 0; to: 200
                            value: root.clock ? root.clock.shieldCm : 50
                            onMoved: if (root.clock) root.clock.shieldCm = value
                        }
                        Text {
                            text: (root.clock ? root.clock.shieldCm : 50).toFixed(0) + " cm"
                            color: Theme.textPrimary
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.typeSm
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Canvas {
                        id: doseChart
                        width: parent.width
                        height: parent.height - 100
                        antialiasing: true

                        function paintCurve() {
                            requestPaint()
                        }

                        Connections {
                            target: root.clock
                            function onSnapshotChanged() { doseChart.requestPaint() }
                        }
                        onWidthChanged: requestPaint()
                        onHeightChanged: requestPaint()

                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.reset()
                            ctx.fillStyle = Theme.bg1
                            ctx.fillRect(0, 0, width, height)
                            if (!root.clock || !root.clock.doseR || root.clock.doseR.length < 2) {
                                ctx.fillStyle = Theme.textMuted
                                ctx.font = Theme.typeSm + "px sans-serif"
                                ctx.fillText("Waiting for radiation snapshot…", 12, height / 2)
                                return
                            }
                            var xs = root.clock.doseR
                            var yg = root.clock.doseGamma
                            var yn = root.clock.doseNeutron
                            var maxY = 1e-9
                            for (var i = 0; i < yg.length; ++i) {
                                maxY = Math.max(maxY, Number(yg[i]), Number(yn[i]))
                            }
                            var padL = 48, padR = 12, padT = 12, padB = 28
                            var w = width - padL - padR
                            var h = height - padT - padB
                            var maxX = Number(xs[xs.length - 1])

                            // axes
                            ctx.strokeStyle = Theme.stroke
                            ctx.beginPath()
                            ctx.moveTo(padL, padT)
                            ctx.lineTo(padL, padT + h)
                            ctx.lineTo(padL + w, padT + h)
                            ctx.stroke()

                            function drawSeries(ys, color) {
                                ctx.beginPath()
                                for (var j = 0; j < ys.length; ++j) {
                                    var x = padL + (Number(xs[j]) / maxX) * w
                                    var y = padT + h - (Number(ys[j]) / maxY) * h
                                    if (j === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                                }
                                ctx.strokeStyle = color
                                ctx.lineWidth = 2
                                ctx.stroke()
                            }
                            drawSeries(yg, Theme.accent)
                            drawSeries(yn, Theme.warn)

                            ctx.fillStyle = Theme.textMuted
                            ctx.font = Theme.typeXs + "px monospace"
                            ctx.fillText(maxY.toFixed(2) + " µSv/h", 4, padT + 10)
                            ctx.fillText("0", 8, padT + h)
                            ctx.fillText(maxX.toFixed(0) + " m", padL + w - 28, height - 8)

                            ctx.fillStyle = Theme.accent
                            ctx.fillText("γ", padL + 8, padT + 14)
                            ctx.fillStyle = Theme.warn
                            ctx.fillText("n", padL + 28, padT + 14)
                        }
                    }

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "Ḣ_γ = Σ A·Γ·B·e^{−μx}/r² ·  Ḣ_n ∝ P·e^{−μ_n x}/r²  ·  see docs/theory/radiation-edu.md"
                        color: Theme.textMuted
                        font.pixelSize: Theme.typeXs
                    }
                }
            }
        }
    }
}
