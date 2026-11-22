pragma Singleton
import QtQuick

QtObject {
    id: root

    property bool dark: true
    property int fontScale: 100 // accessibility %

    // Surfaces — graphite / navy (industrial), not purple-gradient defaults
    readonly property color bg0: dark ? "#070B14" : "#EEF1F6"
    readonly property color bg1: dark ? "#0E1524" : "#FFFFFF"
    readonly property color bg2: dark ? "#162033" : "#E2E8F2"
    readonly property color bgCard: dark ? "#121A2C" : "#FFFFFF"
    readonly property color stroke: dark ? "#243149" : "#C9D2E3"
    readonly property color strokeStrong: dark ? "#3A4D6E" : "#A8B4C8"

    readonly property color textPrimary: dark ? "#E7EEF8" : "#101828"
    readonly property color textMuted: dark ? "#8A9BB5" : "#5B6B84"
    readonly property color textInverse: dark ? "#0B1220" : "#F8FAFC"

    // Accents: cyan normal, amber warn, red alarm, green safe
    readonly property color accent: "#2EC4B6"
    readonly property color accentDim: "#1B7A72"
    readonly property color warn: "#E9A319"
    readonly property color alarm: "#E23B4A"
    readonly property color safe: "#2F9E8F"
    readonly property color masterAlarm: "#FF5A75"

    // Colorblind-safe map anchors (viridis-like)
    readonly property color mapLow: "#440154"
    readonly property color mapMid: "#21918c"
    readonly property color mapHigh: "#fde725"

    // 8 px grid
    readonly property int space1: 4
    readonly property int space2: 8
    readonly property int space3: 16
    readonly property int space4: 24
    readonly property int space5: 32
    readonly property int space6: 40

    readonly property int radiusCard: 20
    readonly property int radiusCtl: 12
    readonly property int radiusPill: 999

    readonly property int railWidth: 76
    readonly property int topBarHeight: 56
    readonly property int transportHeight: 68
    readonly property int bannerHeight: 28

    readonly property string fontUi: "IBM Plex Sans"
    readonly property string fontMono: "IBM Plex Mono"

    function px(n) { return Math.round(n * fontScale / 100) }
    readonly property int typeXs: px(11)
    readonly property int typeSm: px(13)
    readonly property int typeMd: px(15)
    readonly property int typeLg: px(20)
    readonly property int typeXl: px(26)
    readonly property int typeKpi: px(30)

    readonly property int durFast: 160
    readonly property int durMed: 240
    readonly property int durSlow: 300

    readonly property real ratedMWth: 3000.0 // educational; see docs/ui/rated-power.md

    function toggleTheme() { dark = !dark }
    function bumpFont(delta) {
        fontScale = Math.min(140, Math.max(85, fontScale + delta))
    }
}
