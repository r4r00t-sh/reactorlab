import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ReactorLab.ControlCentre

Item {
    id: shell
    property var clock: null

    Rectangle {
        id: banner
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: (shell.clock && shell.clock.nuclearDataPlaceholders) ? 44 : Theme.bannerHeight
        color: Theme.warn
        z: 20
        Column {
            anchors.centerIn: parent
            width: parent.width - 16
            spacing: 2
            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: "EDUCATIONAL CONTROL CENTRE — AHWR-style representative · not safety-grade · P=1 → 3000 MWₜₕ"
                color: "#1a1200"
                font.pixelSize: 12
                font.weight: Font.DemiBold
            }
            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                visible: shell.clock && shell.clock.nuclearDataPlaceholders
                text: shell.clock && shell.clock.placeholderNote
                      ? shell.clock.placeholderNote
                      : "Unresolved nuclear-data markers active"
                color: "#4a2000"
                font.pixelSize: 11
                font.weight: Font.DemiBold
            }
        }
    }

    TopBar {
        id: top
        anchors.top: banner.bottom
        anchors.left: rail.right
        anchors.right: parent.right
        clock: shell.clock
    }

    NavRail {
        id: rail
        anchors.top: banner.bottom
        anchors.bottom: transport.top
        anchors.left: parent.left
        currentIndex: LayoutStore.navIndex
        onNavigated: function (i) { pages.currentIndex = Math.min(i, pages.count - 1) }
    }

    TransportBar {
        id: transport
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clock: shell.clock
    }

    StackLayout {
        id: pages
        anchors.top: top.bottom
        anchors.bottom: transport.top
        anchors.left: rail.right
        anchors.right: parent.right
        anchors.margins: Theme.space3
        currentIndex: LayoutStore.navIndex

        OverviewLayout { clock: shell.clock }
        OperationsLayout { clock: shell.clock }
        AnalysisLayout { clock: shell.clock }
        RadiationLayout { clock: shell.clock }
        ThermalLayout { clock: shell.clock }
        ScenariosLayout { clock: shell.clock }
        AlarmsLayout { clock: shell.clock }
        LogsLayout { clock: shell.clock }
        Item {
            Column {
                anchors.centerIn: parent
                spacing: Theme.space3
                Text {
                    text: "Settings"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.typeXl
                    font.weight: Font.DemiBold
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Row {
                    spacing: Theme.space2
                    anchors.horizontalCenter: parent.horizontalCenter
                    Button {
                        text: Theme.dark ? "Switch to light" : "Switch to dark"
                        onClicked: Theme.toggleTheme()
                    }
                    Button { text: "Font −"; onClicked: Theme.bumpFont(-5) }
                    Button { text: "Font +"; onClicked: Theme.bumpFont(5) }
                }
                Text {
                    text: "Font scale " + Theme.fontScale + "% · rated power " + Theme.ratedMWth + " MWₜₕ"
                    color: Theme.textMuted
                    font.pixelSize: Theme.typeSm
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    Connections {
        target: LayoutStore
        function onNavIndexChanged() { pages.currentIndex = LayoutStore.navIndex }
    }
}
