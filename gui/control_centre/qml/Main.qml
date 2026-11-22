import QtQuick
import QtQuick.Controls
import ReactorLab.ControlCentre

ApplicationWindow {
    id: win
    width: 1440
    height: 900
    minimumWidth: 1280
    minimumHeight: 720
    visible: true
    title: "ReactorLab Control Centre " + (typeof AppVersion !== "undefined" ? AppVersion : "")
    color: Theme.bg0

    // `Engine` is injected from C++ (EngineController). Fallback DemoClock for qml-only runs.
    property var live: typeof Engine !== "undefined" ? Engine : null

    DemoClock {
        id: demo
    }

    Shell {
        anchors.fill: parent
        clock: win.live !== null ? win.live : demo
    }

    Behavior on color { ColorAnimation { duration: Theme.durMed } }
}
