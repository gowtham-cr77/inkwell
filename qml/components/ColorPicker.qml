import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    padding: 12
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property color selectedColor: "#000000"
    signal colorSelected(color col)

    // Palette: Noteastic / GoodNotes-inspired curated vibrant & natural tones
    readonly property var paletteColors: [
        "#000000", "#1E293B", "#475569", "#94A3B8",
        "#DC2626", "#EA580C", "#D97706", "#CA8A04",
        "#16A34A", "#059669", "#0D9488", "#0891B2",
        "#0284C7", "#2563EB", "#4F46E5", "#7C3AED",
        "#9333EA", "#C026D3", "#DB2777", "#E11D48"
    ]

    background: Rectangle {
        color: themeManager.surface
        radius: 10
        border.color: themeManager.outline
        border.width: 1
        layer.enabled: true
    }

    contentItem: ColumnLayout {
        spacing: 12

        Text {
            text: "Select Color"
            color: themeManager.onSurface
            font.bold: true
            font.pixelSize: 13
        }

        // Color Swatch Grid
        GridLayout {
            columns: 5
            rowSpacing: 8
            columnSpacing: 8

            Repeater {
                model: root.paletteColors
                delegate: Rectangle {
                    required property string modelData
                    width: 28
                    height: 28
                    radius: 14
                    color: modelData
                    border.color: root.selectedColor === modelData ? "#FFFFFF" : Qt.darker(modelData, 1.2)
                    border.width: root.selectedColor === modelData ? 3 : 1

                    scale: mouseArea.containsMouse ? 1.15 : 1.0
                    Behavior on scale { NumberAnimation { duration: 100 } }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.selectedColor = parent.modelData
                            root.colorSelected(parent.modelData)
                            root.close()
                        }
                    }
                }
            }
        }

        // Custom Hex input row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                width: 24
                height: 24
                radius: 4
                color: root.selectedColor
                border.color: themeManager.outline
            }

            TextField {
                id: hexField
                Layout.fillWidth: true
                placeholderText: "#RRGGBB"
                text: root.selectedColor.toString()
                color: themeManager.onSurface
                background: Rectangle {
                    color: themeManager.surfaceVariant
                    radius: 6
                    border.color: hexField.activeFocus ? themeManager.primary : themeManager.outline
                }
                onAccepted: {
                    var c = Qt.color(text)
                    if (c.toString() !== "#00000000") {
                        root.selectedColor = c
                        root.colorSelected(c)
                        root.close()
                    }
                }
            }

            Button {
                text: "Apply"
                onClicked: {
                    var c = Qt.color(hexField.text)
                    if (c.toString() !== "#00000000") {
                        root.selectedColor = c
                        root.colorSelected(c)
                        root.close()
                    }
                }
            }
        }
    }
}
