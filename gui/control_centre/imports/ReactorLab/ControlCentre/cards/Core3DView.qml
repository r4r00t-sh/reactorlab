import QtQuick
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Helpers
import ReactorLab.ControlCentre

Rectangle {
    id: root
    property var engine: null
    property real sliceFrac: 0.55
    property bool autoRotate: true

    radius: Theme.radiusCard
    color: Theme.bgCard
    border.color: Theme.stroke
    clip: true

    readonly property real powerPct: {
        if (!engine) return 100
        var p = Number(engine.powerPct)
        return isFinite(p) ? p : 100
    }
    readonly property real rodZ: {
        if (!engine) return 0
        var z = Number(engine.rodZ)
        return isFinite(z) ? Math.max(0, Math.min(1, z)) : 0
    }
    readonly property real tfuel: {
        if (!engine) return 900
        var t = Number(engine.tfuel)
        return isFinite(t) ? t : 900
    }
    readonly property bool scrammed: engine && engine.simState === "Scram"
    readonly property bool glOk: Qt.platform.pluginName !== "offscreen"
            && Qt.platform.pluginName !== "minimal"
            && Qt.platform.pluginName !== "vnc"

    function fuelColor() {
        var t = Math.max(0, Math.min(1, (powerPct - 40) / 80))
        // warm amber → cyan peaking with power
        return Qt.rgba(0.15 + 0.55 * (1 - t), 0.55 + 0.25 * t, 0.35 + 0.45 * t, 1)
    }
    function rodColor() {
        return scrammed ? Theme.alarm : Theme.warn
    }

    // Educational lattice positions (half-core x >= 0)
    readonly property var fuelSites: {
        var out = []
        for (var ix = 0; ix <= 3; ++ix) {
            for (var iz = -3; iz <= 3; ++iz) {
                if (ix === 0 && iz === 0) continue
                // skip a few for control channels
                if ((ix === 1 && (iz === 0 || iz === 2 || iz === -2))
                        || (ix === 2 && iz === 1) || (ix === 3 && iz === 0))
                    continue
                out.push({ x: ix * 22, z: iz * 22 })
            }
        }
        return out
    }
    readonly property var rodSites: [
        { x: 22, z: 0 }, { x: 22, z: 44 }, { x: 22, z: -44 },
        { x: 44, z: 22 }, { x: 66, z: 0 }
    ]

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space3
        spacing: Theme.space2

        Row {
            width: parent.width
            spacing: Theme.space2
            Text {
                text: "3D core"
                color: Theme.textPrimary
                font.pixelSize: Theme.typeMd
                font.weight: Font.DemiBold
            }
            Text {
                text: "EDU cutaway · Quick3D"
                color: Theme.warn
                font.pixelSize: Theme.typeXs
                anchors.verticalCenter: parent.verticalCenter
            }
            Item { width: 8; height: 1 }
            Text {
                text: "Spin"
                color: Theme.textMuted
                font.pixelSize: Theme.typeXs
                anchors.verticalCenter: parent.verticalCenter
            }
            Switch {
                checked: root.autoRotate
                onCheckedChanged: root.autoRotate = checked
            }
        }

        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            text: "Schematic half-core. Fuel tint tracks plant power; rods mirror synced rod_z. Not as-built geometry."
            color: Theme.textMuted
            font.pixelSize: Theme.typeXs
        }

        Loader {
            id: stage
            width: parent.width
            height: parent.height - 110
            sourceComponent: root.glOk ? view3dComp : fallbackComp
        }

        Row {
            width: parent.width
            spacing: Theme.space2
            Text {
                text: "Slice"
                color: Theme.textMuted
                font.pixelSize: Theme.typeSm
                anchors.verticalCenter: parent.verticalCenter
            }
            Slider {
                width: parent.width - 80
                from: 0.05; to: 0.95
                value: root.sliceFrac
                onMoved: root.sliceFrac = value
            }
        }
    }

    Component {
        id: fallbackComp
        Rectangle {
            color: Theme.bg1
            border.color: Theme.stroke
            radius: Theme.radiusCtl
            Column {
                anchors.centerIn: parent
                spacing: Theme.space2
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "3D view needs a GL display"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.typeMd
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Offscreen/smoke fallback · power "
                          + root.powerPct.toFixed(0) + "% · rod_z "
                          + root.rodZ.toFixed(2)
                    color: Theme.textMuted
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.typeSm
                }
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 160; height: 120
                    radius: 8
                    color: Theme.bg2
                    border.color: Theme.stroke
                    // simple 2D cutaway glyph
                    Rectangle {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width * 0.55
                        height: parent.height * 0.75
                        radius: 6
                        color: root.fuelColor()
                        opacity: 0.85
                    }
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 8
                        width: 10
                        height: parent.height * 0.75 * root.rodZ
                        color: root.rodColor()
                    }
                }
            }
        }
    }

    Component {
        id: view3dComp
        Item {
            View3D {
                id: view
                anchors.fill: parent

                environment: SceneEnvironment {
                    backgroundMode: SceneEnvironment.Color
                    clearColor: Theme.bg1
                    antialiasingMode: SceneEnvironment.MSAA
                    antialiasingQuality: SceneEnvironment.High
                }

                Node {
                    id: origin

                    PerspectiveCamera {
                        id: camera
                        z: 320
                        y: 90
                        x: 140
                        clipNear: 1
                        clipFar: 2000
                        eulerRotation.x: -18
                        eulerRotation.y: 32
                    }

                    DirectionalLight {
                        eulerRotation.x: -40
                        eulerRotation.y: 35
                        brightness: 1.1
                        ambientColor: Qt.rgba(0.25, 0.28, 0.35, 1)
                    }

                    Node {
                        id: core
                        y: 0

                        // Vessel shell (half-ish translucent)
                        Model {
                            source: "#Cylinder"
                            scale: Qt.vector3d(1.55, 2.2, 1.55)
                            materials: [
                                PrincipledMaterial {
                                    baseColor: Qt.rgba(0.55, 0.62, 0.72, 0.22)
                                    metalness: 0.2
                                    roughness: 0.55
                                    alphaMode: PrincipledMaterial.Blend
                                }
                            ]
                        }

                        // Cut face
                        Model {
                            source: "#Cube"
                            scale: Qt.vector3d(0.02, 2.05, 1.45)
                            x: -1
                            materials: [
                                PrincipledMaterial {
                                    baseColor: root.fuelColor()
                                    metalness: 0.05
                                    roughness: 0.7
                                    emissiveFactor: Qt.vector3d(
                                        0.15 * root.powerPct / 120,
                                        0.2 * root.powerPct / 120,
                                        0.18 * root.powerPct / 120)
                                }
                            ]
                        }

                        // Fuel columns
                        Repeater3D {
                            model: root.fuelSites
                            delegate: Model {
                                required property var modelData
                                source: "#Cylinder"
                                x: modelData.x
                                z: modelData.z
                                scale: Qt.vector3d(0.18, 1.9, 0.18)
                                materials: [
                                    PrincipledMaterial {
                                        baseColor: root.fuelColor()
                                        metalness: 0.1
                                        roughness: 0.45
                                        emissiveFactor: Qt.vector3d(
                                            0.08 * root.powerPct / 100,
                                            0.12 * root.powerPct / 100,
                                            0.1 * root.powerPct / 100)
                                    }
                                ]
                            }
                        }

                        // Control rods — drop from top with rod_z
                        Repeater3D {
                            model: root.rodSites
                            delegate: Model {
                                required property var modelData
                                source: "#Cylinder"
                                x: modelData.x
                                z: modelData.z
                                property real rodLen: 1.9 * Math.max(0.05, root.rodZ)
                                scale: Qt.vector3d(0.12, rodLen, 0.12)
                                y: 95 - (190 * Math.max(0.05, root.rodZ)) / 2
                                materials: [
                                    PrincipledMaterial {
                                        baseColor: root.rodColor()
                                        metalness: 0.35
                                        roughness: 0.35
                                    }
                                ]
                                Behavior on rodLen {
                                    NumberAnimation { duration: Theme.durMed; easing.type: Easing.OutCubic }
                                }
                                Behavior on y {
                                    NumberAnimation { duration: Theme.durMed; easing.type: Easing.OutCubic }
                                }
                            }
                        }

                        // Horizontal slice plane
                        Model {
                            id: slicePlane
                            source: "#Cube"
                            scale: Qt.vector3d(1.5, 0.02, 1.5)
                            y: -95 + root.sliceFrac * 190
                            materials: [
                                PrincipledMaterial {
                                    baseColor: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.35)
                                    alphaMode: PrincipledMaterial.Blend
                                    metalness: 0.0
                                    roughness: 1.0
                                }
                            ]
                            Behavior on y {
                                NumberAnimation { duration: Theme.durFast; easing.type: Easing.OutCubic }
                            }
                        }
                    }

                    NumberAnimation {
                        id: spinAnim
                        target: core
                        property: "eulerRotation.y"
                        from: 0; to: 360
                        duration: 28000
                        loops: Animation.Infinite
                        running: root.autoRotate
                    }
                }

                OrbitCameraController {
                    id: orbit
                    anchors.fill: parent
                    origin: origin
                    camera: camera
                }

                Connections {
                    target: orbit
                    function onInputsNeedProcessingChanged() {
                        if (orbit.inputsNeedProcessing)
                            root.autoRotate = false
                    }
                }
            }

            // Live HUD
            Column {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 8
                spacing: 2
                Text {
                    text: "P " + root.powerPct.toFixed(0) + "% · Tf "
                          + root.tfuel.toFixed(0) + " K"
                    color: Theme.textPrimary
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.typeXs
                }
                Text {
                    text: "rod_z " + root.rodZ.toFixed(2)
                          + (root.scrammed ? " · SCRAM" : "")
                    color: root.scrammed ? Theme.alarm : Theme.warn
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.typeXs
                }
            }
        }
    }
}
