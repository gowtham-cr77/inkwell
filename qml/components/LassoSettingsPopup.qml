import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    padding: 14
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property bool selectPen: true
    property bool selectHighlighter: true
    property bool selectShapes: true
    property int selectedCount: 0

    signal togglePen(bool val)
    signal toggleHighlighter(bool val)
    signal toggleShapes(bool val)
    signal deleteTriggered()
    signal copyTriggered()
    signal pasteTriggered()
    signal ocrTriggered()
    signal recolorTriggered(color c)

    background: Rectangle {
        color: themeManager.surface
        radius: 12
        border.color: themeManager.outline
        border.width: 1
    }

    contentItem: ColumnLayout {
        spacing: 12
        width: 240

        Text {
            text: "Lasso Selection"
            color: themeManager.onSurface
            font.bold: true
            font.pixelSize: 14
        }

        Text {
            text: root.selectedCount > 0 ? (root.selectedCount + " item(s) selected") : "Circle elements to select"
            color: root.selectedCount > 0 ? themeManager.primary : themeManager.onSurfaceVariant
            font.pixelSize: 12
        }

        // Actions if items selected
        if (root.selectedCount > 0) {
            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true

                RowLayout {
                    spacing: 6
                    Button {
                        text: "Copy"
                        Layout.fillWidth: true
                        onClicked: root.copyTriggered()
                    }
                    Button {
                        text: "Delete"
                        Layout.fillWidth: true
                        onClicked: root.deleteTriggered()
                    }
                }

                Button {
                    text: "Recognize Handwriting (OCR)"
                    Layout.fillWidth: true
                    highlighted: true
                    onClicked: root.ocrTriggered()
                }

                // Recolor row
                RowLayout {
                    spacing: 6
                    Text { text: "Recolor:"; font.pixelSize: 11; color: themeManager.onSurfaceVariant }
                    Repeater {
                        model: ["#000000", "#2563EB", "#DC2626", "#16A34A", "#9333EA"]
                        delegate: Rectangle {
                            required property string modelData
                            width: 22
                            height: 22
                            radius: 11
                            color: modelData
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.recolorTriggered(parent.modelData)
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            height: 1
            Layout.fillWidth: true
            color: themeManager.outline
        }

        Text {
            text: "Select Target Types:"
            font.bold: true
            font.pixelSize: 12
            color: themeManager.onSurface
        }

        CheckBox {
            text: "Pen Strokes"
            checked: root.selectPen
            onToggled: root.togglePen(checked)
        }
        CheckBox {
            text: "Highlighter Strokes"
            checked: root.selectHighlighter
            onToggled: root.toggleHighlighter(checked)
        }
        CheckBox {
            text: "Shapes & Geometry"
            checked: root.selectShapes
            onToggled: root.toggleShapes(checked)
        }
    }
}
