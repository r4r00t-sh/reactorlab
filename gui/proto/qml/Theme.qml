pragma Singleton
import QtQuick

QtObject {
    id: root

    // --- Mode ---
    property bool dark: true

    // --- Colors (design tokens) ---
    readonly property color bg0: dark ? "#0B1220" : "#F4F6FA"
    readonly property color bg1: dark ? "#121A2B" : "#FFFFFF"
    readonly property color bg2: dark ? "#182338" : "#E8ECF4"
    readonly property color bgCard: dark ? "#151E32" : "#FFFFFF"
    readonly property color stroke: dark ? "#2A3650" : "#D0D7E6"
    readonly property color textPrimary: dark ? "#E8EEF9" : "#121826"
    readonly property color textMuted: dark ? "#8B9BB8" : "#5A6A85"
    readonly property color accent: "#2EC4B6"       // cyan/teal — normal
    readonly property color accentDim: "#1A7F76"
    readonly property color warn: "#F0A202"         // amber
    readonly property color alarm: "#E63946"        // red
    readonly property color safe: "#2A9D8F"         // green
    readonly property color masterAlarm: "#FF4D6D"

    // Colorblind-safe sequential (viridis-ish endpoints for sparklines)
    readonly property color mapLow: "#440154"
    readonly property color mapMid: "#21918c"
    readonly property color mapHigh: "#fde725"

    // --- Radii / spacing (8 px grid) ---
    readonly property int space1: 4
    readonly property int space2: 8
    readonly property int space3: 16
    readonly property int space4: 24
    readonly property int space5: 32
    readonly property int radiusCard: 20
    readonly property int radiusCtl: 12
    readonly property int railWidth: 72
    readonly property int topBarHeight: 56
    readonly property int transportHeight: 64

    // --- Type ---
    readonly property string fontUi: "IBM Plex Sans"
    readonly property string fontMono: "IBM Plex Mono"
    readonly property int fontScale: 100 // % — accessibility
    function px(n) { return Math.round(n * fontScale / 100) }
    readonly property int typeXs: px(11)
    readonly property int typeSm: px(13)
    readonly property int typeMd: px(15)
    readonly property int typeLg: px(20)
    readonly property int typeXl: px(28)
    readonly property int typeKpi: px(32)

    // --- Elevation / motion ---
    readonly property real elevCard: dark ? 0.35 : 0.12
    readonly property int durFast: 160
    readonly property int durMed: 240
    readonly property int durSlow: 300

    function toggleTheme() { dark = !dark }
}
