import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    padding: 14
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property color activeLaserColor: "#EF4444"
    property real activeLaserWidth: 6.0
    property int activeLaserMode: 1 // 0=Dot, 1=Line

    signal colorChanged(color c)
    signal widthChanged(real w)
    signal modeChanged(int m)

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
            text: "Presentation Laser"
            color: themeManager.onSurface
            font.bold: true
            font.pixelSize: 14
        }

        Text {
            text: "Laser pointer leaves temporary glowing trails that fade automatically without modifying the notebook."
            color: themeManager.onSurfaceVariant
            font.pixelSize: 11
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        // Mode: Dot vs Line
        RowLayout {
            spacing: 8
            Layout.fillWidth: true

            Button {
                text: "Trail Line"
                Layout.fillWidth: true
                highlighted: root.activeLaserMode === 1
                onClicked: {
                    root.activeLaserMode = 1
                    root.modeChanged(1)
                }
            }
            Button {
                text: "Dot Only"
                Layout.fillWidth: true
                highlighted: root.activeLaserMode === 0
                onClicked: {
                    root.activeLaserMode = 0
                    root.modeChanged(0)
                }
            }
        }

        // Color selection
        RowLayout {
            spacing: 8
            Repeater {
                model: ["#EF4444", "#10B981", "#3B82F6", "#F59E0B", "#EC4899"]
                delegate: Rectangle {
                    required property string modelData
                    width: 30
                    height: 30
                    radius: 15
                    color: modelData
                    border.color: root.activeLaserColor === modelData ? themeManager.primary : "#94A3B8"
                    border.width: root.activeLaserColor === modelData ? 3 : 1

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.activeLaserColor = parent.modelData
                            root.colorChanged(parent.modelData)
                        }
                    }
                }
            }
        }
    }
}
