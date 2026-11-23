import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: "Inkwell Settings"
    modal: true
    width: 440
    standardButtons: Dialog.Close

    background: Rectangle {
        color: themeManager.surface
        radius: 12
        border.color: themeManager.outline
    }

    contentItem: ColumnLayout {
        spacing: 16

        Text {
            text: "Stylus & Drawing Engine"
            color: themeManager.onSurface
            font.bold: true
            font.pixelSize: 14
        }

        CheckBox {
            text: "Enable One Euro Jitter Filter (Smooth Inking)"
            checked: true
        }

        CheckBox {
            text: "Hardware Pressure Sensitivity (Huion / Wacom / MPP)"
            checked: true
        }

        Text {
            text: "Interface & Appearance"
            color: themeManager.onSurface
            font.bold: true
            font.pixelSize: 14
        }

        RowLayout {
            Text { text: "Theme Mode:"; color: themeManager.onSurfaceVariant; font.pixelSize: 13 }
            Button {
                text: themeManager.isDark ? "🌙 Dark Mode" : "☀️ Light Mode"
                onClicked: themeManager.toggleTheme()
            }
        }

        Rectangle {
            height: 1
            Layout.fillWidth: true
            color: themeManager.outline
        }

        Text {
            text: "About Inkwell"
            color: themeManager.onSurface
            font.bold: true
            font.pixelSize: 14
        }

        Text {
            text: "Inkwell v0.1.0\nDistraction-free handwritten digital notebook crafted natively for Fedora Linux & KDE Plasma.\nBuilt with C++17 and Qt 6."
            color: themeManager.onSurfaceVariant
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
