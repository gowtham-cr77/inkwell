import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: layout.implicitWidth + 16
    height: 40
    radius: 20
    color: themeManager.surface
    border.color: themeManager.outline
    border.width: 1

    property real zoomLevel: 1.0
    signal zoomInClicked()
    signal zoomOutClicked()
    signal zoomFitClicked()

    RowLayout {
        id: layout
        anchors.centerIn: parent
        spacing: 6

        ToolButton {
            text: "−"
            font.bold: true
            font.pixelSize: 16
            onClicked: root.zoomOutClicked()
        }

        Text {
            text: Math.round(root.zoomLevel * 100) + "%"
            color: themeManager.onSurface
            font.pixelSize: 12
            font.bold: true
            Layout.preferredWidth: 42
            horizontalAlignment: Text.AlignHCenter

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.zoomFitClicked()
            }
        }

        ToolButton {
            text: "+"
            font.bold: true
            font.pixelSize: 16
            onClicked: root.zoomInClicked()
        }

        ToolSeparator {
            contentItem: Rectangle { implicitWidth: 1; color: themeManager.outline }
        }

        ToolButton {
            text: "⛶"
            ToolTip.text: "Fit to Viewport"
            ToolTip.visible: hovered
            onClicked: root.zoomFitClicked()
        }
    }
}
