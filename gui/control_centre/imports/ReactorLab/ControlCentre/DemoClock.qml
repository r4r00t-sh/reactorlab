import QtQuick

// Fallback when Engine context property is absent (qml-only runs).
QtObject {
    id: clock
    property bool running: false
    property real simTime: 0
    property real speed: 1
    property string scenarioName: "— idle —"
    property string reactorName: "ReactorLab · EDU-1"
    property string simState: running ? "Running" : "Paused"
    property bool masterAlarm: false
    property real powerPct: 100
    property real powerMW: powerPct / 100.0 * 3000.0
    property real rhoPcm: -12
    property real periodS: 95
    property real keff: 0.99988
    property real tfuel: 905
    property real tcool: 582
    property real mdot: 29000
    property real xenonWorthPcm: -2380
    property real rodZ: 0.2
    property var sparkPower: []
    property var sparkRho: []
    property var trendT: []
    property var trendP: []
    property var trendRho: []
    property var trendTf: []
    property var trendTc: []
    property var trendX: []
    property int mapNx: 0
    property int mapNy: 0
    property var mapPower: []
    property var mapFlux: []
    property var mapTfuel: []
    property var mapTcoolAxial: []
    property string mapNote: "demo — no TWIGL form"
    property var radNames: []
    property var radKinds: []
    property var radActivityBq: []
    property real radAlphaBq: 0
    property real radBetaBq: 0
    property real radGammaBq: 0
    property real shieldCm: 50
    property var doseR: []
    property var doseGamma: []
    property var doseNeutron: []
    property string radNote: "Educational estimates — not for safety or dose assessment."
    property var alarmIds: []
    property var alarmMsgs: []
    property var alarmSeverities: []
    property int alarmUnacked: 0
    property var eventLog: []
    property real mdotProxy: 29000
    property string lastExport: ""
    property var scenarioIds: ["manual", "rod_step", "scram_xe", "doppler"]
    property var scenarioLabels: ["Manual", "Rod step", "Scram → Xe", "Doppler"]
    property string exportStatus: ""
    function setScenario(id) { scenarioName = id }
    function acknowledgeAlarms() { alarmUnacked = 0; masterAlarm = false }
    function exportCsv(path) { exportStatus = "demo skip: " + path }
    signal trendsChanged()
    signal snapshotChanged()

    function play() { running = true; simState = "Running" }
    function pause() { running = false; if (simState !== "Scram") simState = "Paused" }
    function scram() {
        rodZ = 1
        rhoPcm = -4800
        masterAlarm = true
        simState = "Scram"
        running = false
    }
    function reset() {
        simTime = 0; powerPct = 100; rhoPcm = -12; periodS = 95
        keff = 0.99988; tfuel = 905; tcool = 582; mdot = 29000
        rodZ = 0.2; masterAlarm = false; running = false; simState = "Paused"
        sparkPower = []; sparkRho = []
    }

    function _push(a, v, n) {
        var x = a.slice(); x.push(v)
        if (x.length > n) x = x.slice(x.length - n)
        return x
    }

    property Timer t: Timer {
        interval: 50; running: true; repeat: true
        onTriggered: {
            if (!clock.running) return
            clock.simTime += 0.05 * clock.speed
            clock.powerPct += (100 - clock.rodZ * 15 - clock.powerPct) * 0.04
                + Math.sin(clock.simTime * 0.15) * 0.08
            clock.rhoPcm = (0.2 - clock.rodZ) * 350 + Math.sin(clock.simTime * 0.05) * 8
            clock.tfuel = 900 + (clock.powerPct - 100) * 1.5
            clock.tcool = 580 + (clock.powerPct - 100) * 0.3
            clock.sparkPower = clock._push(clock.sparkPower, clock.powerPct, 36)
            clock.sparkRho = clock._push(clock.sparkRho, clock.rhoPcm, 36)
        }
    }
}
