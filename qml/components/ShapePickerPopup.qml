import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    padding: 14
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property int activeShapeType: 0
    property bool activeFilled: false
    property bool activeSnapAngles: false

    signal shapeSelected(int type)
    signal filledToggled(bool filled)
    signal snapAnglesToggled(bool snap)

    background: Rectangle {
        color: themeManager.surface
        radius: 12
        border.color: themeManager.outline
        border.width: 1
    }

    contentItem: ColumnLayout {
        spacing: 12
        width: 220

        Text {
            text: "Geometry & Shapes"
            color: themeManager.onSurface
            font.bold: true
            font.pixelSize: 14
        }

        GridLayout {
            columns: 3
            rowSpacing: 8
            columnSpacing: 8

            Repeater {
                model: [
                    { name: "Line", type: 0, icon: "―" },
                    { name: "Arrow", type: 1, icon: "➔" },
                    { name: "Rectangle", type: 2, icon: "▭" },
                    { name: "Circle", type: 3, icon: "○" },
                    { name: "Triangle", type: 4, icon: "△" },
                    { name: "Ellipse", type: 5, icon: "⬭" }
                ]

                delegate: Rectangle {
                    required property var modelData
                    width: 60
                    height: 48
                    radius: 8
                    color: root.activeShapeType === modelData.type ? themeManager.primary : themeManager.surfaceVariant

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            text: parent.parent.modelData.icon
                            font.pixelSize: 18
                            color: root.activeShapeType === parent.parent.modelData.type ? "#FFFFFF" : themeManager.onSurface
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Text {
                            text: parent.parent.modelData.name
                            font.pixelSize: 10
                            color: root.activeShapeType === parent.parent.modelData.type ? "#FFFFFF" : themeManager.onSurfaceVariant
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.activeShapeType = parent.modelData.type
                            root.shapeSelected(parent.modelData.type)
                        }
                    }
                }
            }
        }

        // Toggles
        CheckBox {
            text: "Snap Angles (45° / 90°)"
            checked: root.activeSnapAngles
            onToggled: {
                root.activeSnapAngles = checked
                root.snapAnglesToggled(checked)
            }
        }

        CheckBox {
            text: "Fill Shape"
            checked: root.activeFilled
            onToggled: {
                root.activeFilled = checked
                root.filledToggled(checked)
            }
        }
    }
}
