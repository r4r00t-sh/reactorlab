import QtQuick

Canvas {
    id: root
    property var values: []
    property color stroke: "#2EC4B6"
    property color fill: Qt.rgba(stroke.r, stroke.g, stroke.b, 0.15)
    antialiasing: true

    onValuesChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()
        if (!values || values.length < 2) return
        var min = values[0], max = values[0]
        for (var i = 1; i < values.length; ++i) {
            min = Math.min(min, values[i])
            max = Math.max(max, values[i])
        }
        if (max === min) { max += 1; min -= 1 }
        ctx.beginPath()
        for (i = 0; i < values.length; ++i) {
            var x = i / (values.length - 1) * width
            var y = height - (values[i] - min) / (max - min) * (height - 2) - 1
            if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
        }
        ctx.strokeStyle = stroke
        ctx.lineWidth = 1.5
        ctx.stroke()
        ctx.lineTo(width, height)
        ctx.lineTo(0, height)
        ctx.closePath()
        ctx.fillStyle = fill
        ctx.fill()
    }
}
