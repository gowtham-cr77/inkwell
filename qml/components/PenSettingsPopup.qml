import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    padding: 14
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property bool isHighlighter: false
    property color activeColor: "#000000"
    property real activeWidth: 2.5
    property real activeOpacity: 1.0

    signal colorChanged(color c)
    signal widthChanged(real w)
    signal opacityChanged(real op)

    readonly property var penPresets: [
        "#000000", "#1E40AF", "#DC2626", "#047857", "#6D28D9", "#D97706"
    ]
    readonly property var highlighterPresets: [
        "#FEF08A", "#BBF7D0", "#BAE6FD", "#FED7AA", "#FBCFE8", "#E9D5FF"
    ]

    background: Rectangle {
        color: themeManager.surface
        radius: 12
        border.color: themeManager.outline
        border.width: 1
        layer.enabled: true
    }

    contentItem: ColumnLayout {
        spacing: 14
        width: 240

        // Title
        Text {
            text: root.isHighlighter ? "Highlighter Settings" : "Pen Settings"
            color: themeManager.onSurface
            font.bold: true
            font.pixelSize: 14
        }

        // Stroke Width Slider with Live Preview
        ColumnLayout {
            spacing: 6
            Layout.fillWidth: true

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Thickness"
                    color: themeManager.onSurfaceVariant
                    font.pixelSize: 12
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: root.activeWidth.toFixed(1) + " px"
                    color: themeManager.primary
                    font.bold: true
                    font.pixelSize: 12
                }
            }

            Slider {
                id: widthSlider
                Layout.fillWidth: true
                from: root.isHighlighter ? 5.0 : 0.5
                to: root.isHighlighter ? 40.0 : 25.0
                value: root.activeWidth
                stepSize: 0.5
                onMoved: {
                    root.activeWidth = value
                    root.widthChanged(value)
                }
            }

            // Live Nib Preview
            Rectangle {
                Layout.fillWidth: true
                height: 36
                radius: 6
                color: themeManager.surfaceVariant

                Rectangle {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - 20, Math.max(2, root.activeWidth * 2))
                    height: Math.min(26, Math.max(2, root.activeWidth))
                    radius: root.isHighlighter ? 1 : height / 2
                    color: root.activeColor
                    opacity: root.isHighlighter ? 0.5 : root.activeOpacity
                }
            }
        }

        // Quick Swatches
        ColumnLayout {
            spacing: 6
            Layout.fillWidth: true

            Text {
                text: "Quick Colors"
                color: themeManager.onSurfaceVariant
                font.pixelSize: 12
            }

            RowLayout {
                spacing: 8
                Repeater {
                    model: root.isHighlighter ? root.highlighterPresets : root.penPresets
                    delegate: Rectangle {
                        required property string modelData
                        width: 28
                        height: 28
                        radius: 14
                        color: modelData
                        border.color: root.activeColor === modelData ? themeManager.primary : "#94A3B8"
                        border.width: root.activeColor === modelData ? 3 : 1

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.activeColor = parent.modelData
                                root.colorChanged(parent.modelData)
                            }
                        }
                    }
                }
            }
        }
    }
}
