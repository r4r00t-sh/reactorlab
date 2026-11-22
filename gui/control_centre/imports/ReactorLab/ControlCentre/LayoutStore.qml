pragma Singleton
import QtQuick

QtObject {
    id: store

    // 0 Overview, 1 Operations, 2 Analysis, 3 Radiation (gated)
    property int preset: 0
    property int navIndex: 0

    readonly property var presetNames: ["Overview", "Operations", "Analysis", "Radiation"]

    function setPreset(i) {
        preset = Math.max(0, Math.min(3, i))
        navIndex = preset
    }

    function setNav(i) {
        navIndex = i
        if (i >= 0 && i <= 3)
            preset = i
    }

    // Persist lightweight prefs (theme handled in Theme; this stores layout)
    property string lastPresetName: "Overview"
    onPresetChanged: lastPresetName = presetNames[preset]
}
