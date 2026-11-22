import QtQuick

// UI-0 ONLY — synthetic demo signals. Not connected to libreactorlab.
QtObject {
    id: eng

    property bool running: false
    property real simTime: 0
    property real speed: 1
    property string scenarioName: "Demo · rod bank A"
    property string reactorName: "ReactorLab · EDU-1"
    property string simState: running ? "Running" : "Paused"
    property bool masterAlarm: powerPct > 118 || tfuel > 1250

    property real powerPct: 100
    property real rhoPcm: 0
    property real periodS: 80
    property real keff: 1.000
    property real tfuel: 900
    property real tcool: 580
    property real mdot: 29000
    property real pressureMPa: 15.5
    property real xenonWorthPcm: -2400
    property real rodZ: 0.35   // 0=out, 1=full in

    property var sparkPower: []
    property var sparkRho: []
    property var trendT: []
    property var trendP: []
    property var trendRho: []
    property var trendTf: []
    property var trendTc: []

    property int selectedParam: 0 // 0 P, 1 rho, 2 Tf, 3 Tc

    function play() { running = true }
    function pause() { running = false }
    function scram() {
        rodZ = 1.0
        rhoPcm = -4500
        masterAlarm = true
        simState = "Scram"
        running = false
    }
    function reset() {
        simTime = 0
        powerPct = 100
        rhoPcm = 0
        periodS = 80
        keff = 1.0
        tfuel = 900
        tcool = 580
        mdot = 29000
        rodZ = 0.35
        masterAlarm = false
        simState = "Paused"
        running = false
        sparkPower = []
        sparkRho = []
        trendT = []; trendP = []; trendRho = []; trendTf = []; trendTc = []
    }

    function _push(arr, v, maxN) {
        var a = arr.slice()
        a.push(v)
        if (a.length > maxN) a = a.slice(a.length - maxN)
        return a
    }

    function tick(dt) {
        if (!running) return
        simTime += dt * speed
        // gentle educational wobble + rod effect
        var target = 100 + (-rhoPcm) * 0.01 - rodZ * 8
        powerPct += (target - powerPct) * 0.05 + (Math.sin(simTime * 0.2) * 0.15)
        rhoPcm = (0.35 - rodZ) * 400 + Math.sin(simTime * 0.07) * 12
        periodS = Math.max(5, 200 / Math.max(0.2, Math.abs(rhoPcm) * 0.05 + 1))
        keff = 1.0 + rhoPcm * 1e-5
        tfuel = 900 + (powerPct - 100) * 1.8
        tcool = 580 + (powerPct - 100) * 0.35 + (1 - mdot / 29000) * 25
        sparkPower = _push(sparkPower, powerPct, 40)
        sparkRho = _push(sparkRho, rhoPcm, 40)
        trendT = _push(trendT, simTime, 240)
        trendP = _push(trendP, powerPct, 240)
        trendRho = _push(trendRho, rhoPcm, 240)
        trendTf = _push(trendTf, tfuel, 240)
        trendTc = _push(trendTc, tcool, 240)
    }

    property Timer clock: Timer {
        interval: 50
        running: true
        repeat: true
        onTriggered: eng.tick(0.05)
    }

    Component.onCompleted: reset()
}
