import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: layout.implicitWidth + 18
    height: 40
    radius: 20
    color: themeManager.surface
    border.color: themeManager.outline
    border.width: 1

    property int currentPage: 0
    property int totalPages: 1

    signal prevClicked()
    signal nextClicked()
    signal addClicked()
    signal deleteClicked()
    signal duplicateClicked()

    RowLayout {
        id: layout
        anchors.centerIn: parent
        spacing: 6

        ToolButton {
            text: "‹"
            font.bold: true
            font.pixelSize: 18
            enabled: root.currentPage > 0
            onClicked: root.prevClicked()
        }

        Text {
            text: (root.currentPage + 1) + " / " + Math.max(1, root.totalPages)
            color: themeManager.onSurface
            font.pixelSize: 12
            font.bold: true
            Layout.preferredWidth: 54
            horizontalAlignment: Text.AlignHCenter
        }

        ToolButton {
            text: "›"
            font.bold: true
            font.pixelSize: 18
            enabled: root.currentPage < root.totalPages - 1
            onClicked: root.nextClicked()
        }

        ToolSeparator {
            contentItem: Rectangle { implicitWidth: 1; color: themeManager.outline }
        }

        ToolButton {
            text: "+ Page"
            font.pixelSize: 11
            font.bold: true
            ToolTip.text: "Add New Page"
            ToolTip.visible: hovered
            onClicked: root.addClicked()
        }

        ToolButton {
            text: "📋"
            font.pixelSize: 12
            ToolTip.text: "Duplicate Current Page"
            ToolTip.visible: hovered
            onClicked: root.duplicateClicked()
        }

        ToolButton {
            text: "🗑"
            font.pixelSize: 12
            enabled: root.totalPages > 1
            ToolTip.text: "Delete Current Page"
            ToolTip.visible: hovered
            onClicked: root.deleteClicked()
        }
    }
}
