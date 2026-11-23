import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    padding: 14
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property int activePaperType: 0 // 0=Blank, 1=Ruled, 2=Grid, 3=Dotted, 4=Cornell
    property color activePaperColor: "#FFFFFF"
    property real activeLineSpacing: 30.0

    signal paperTypeSelected(int type)
    signal paperColorSelected(color c)
    signal lineSpacingChanged(real sp)

    background: Rectangle {
        color: themeManager.surface
        radius: 12
        border.color: themeManager.outline
        border.width: 1
    }

    contentItem: ColumnLayout {
        spacing: 12
        width: 250

        Text {
            text: "Paper Background"
            color: themeManager.onSurface
            font.bold: true
            font.pixelSize: 14
        }

        // Templates grid
        GridLayout {
            columns: 3
            rowSpacing: 8
            columnSpacing: 8

            Repeater {
                model: [
                    { name: "Blank", type: 0, icon: "📄" },
                    { name: "Ruled", type: 1, icon: "📝" },
                    { name: "Grid", type: 2, icon: "▦" },
                    { name: "Dotted", type: 3, icon: "⁘" },
                    { name: "Cornell", type: 4, icon: "📑" }
                ]

                delegate: Rectangle {
                    required property var modelData
                    width: 70
                    height: 54
                    radius: 8
                    color: root.activePaperType === modelData.type ? themeManager.primary : themeManager.surfaceVariant

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            text: parent.parent.modelData.icon
                            font.pixelSize: 18
                            color: root.activePaperType === parent.parent.modelData.type ? "#FFFFFF" : themeManager.onSurface
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Text {
                            text: parent.parent.modelData.name
                            font.pixelSize: 10
                            color: root.activePaperType === parent.parent.modelData.type ? "#FFFFFF" : themeManager.onSurfaceVariant
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.activePaperType = parent.modelData.type
                            root.paperTypeSelected(parent.modelData.type)
                        }
                    }
                }
            }
        }

        // Paper Colors
        Text {
            text: "Paper Tint"
            color: themeManager.onSurfaceVariant
            font.pixelSize: 12
        }

        RowLayout {
            spacing: 8
            Repeater {
                model: [
                    { name: "White", color: "#FFFFFF" },
                    { name: "Cream", color: "#FDFBF7" },
                    { name: "Ivory", color: "#F8F6EB" },
                    { name: "Dark", color: "#22252A" },
                    { name: "Sepia", color: "#EEDBB8" }
                ]
                delegate: Rectangle {
                    required property var modelData
                    width: 32
                    height: 32
                    radius: 16
                    color: modelData.color
                    border.color: root.activePaperColor === modelData.color ? themeManager.primary : "#94A3B8"
                    border.width: root.activePaperColor === modelData.color ? 3 : 1

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.activePaperColor = parent.modelData.color
                            root.paperColorSelected(parent.modelData.color)
                        }
                    }
                }
            }
        }

        // Line Spacing Slider
        if (root.activePaperType > 0) {
            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Line / Grid Size"; font.pixelSize: 11; color: themeManager.onSurfaceVariant }
                    Item { Layout.fillWidth: true }
                    Text { text: root.activeLineSpacing.toFixed(0) + " pt"; font.pixelSize: 11; color: themeManager.primary }
                }

                Slider {
                    Layout.fillWidth: true
                    from: 15.0
                    to: 60.0
                    value: root.activeLineSpacing
                    stepSize: 5.0
                    onMoved: {
                        root.activeLineSpacing = value
                        root.lineSpacingChanged(value)
                    }
                }
            }
        }
    }
}
