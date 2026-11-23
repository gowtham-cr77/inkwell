import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: "Export Notebook to PDF"
    modal: true
    width: 400
    standardButtons: Dialog.Ok | Dialog.Cancel

    property string exportPath: ""
    property bool includeBackground: true
    signal acceptedExport(string path, bool withBg)

    background: Rectangle {
        color: themeManager.surface
        radius: 12
        border.color: themeManager.outline
    }

    contentItem: ColumnLayout {
        spacing: 14

        Text {
            text: "Destination Path:"
            color: themeManager.onSurfaceVariant
            font.pixelSize: 12
        }

        TextField {
            id: pathField
            Layout.fillWidth: true
            text: "exported_notes.pdf"
            color: themeManager.onSurface
            background: Rectangle {
                color: themeManager.surfaceVariant
                radius: 6
                border.color: pathField.activeFocus ? themeManager.primary : themeManager.outline
            }
        }

        CheckBox {
            id: bgCheck
            text: "Include Paper Background & Grid Lines"
            checked: true
        }
    }

    onAccepted: {
        root.acceptedExport(pathField.text, bgCheck.checked)
    }
}
