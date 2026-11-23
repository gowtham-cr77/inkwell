import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../dialogs"

Item {
    id: root

    signal notebookOpened(string notebookId)

    Component.onCompleted: {
        libraryModel.refresh()
    }

    function refresh() {
        libraryModel.refresh()
    }

    SettingsDialog {
        id: settingsDialog
        anchors.centerIn: parent
    }

    Dialog {
        id: newNotebookDialog
        title: "Create New Notebook"
        modal: true
        anchors.centerIn: parent
        width: 320
        standardButtons: Dialog.Ok | Dialog.Cancel

        contentItem: ColumnLayout {
            spacing: 10
            TextField {
                id: nbTitleInput
                Layout.fillWidth: true
                placeholderText: "Notebook Title (e.g., Physics 101)"
                color: themeManager.onSurface
            }
        }
        onAccepted: {
            var title = nbTitleInput.text.trim()
            if (title === "") title = "Untitled Notebook"
            var id = notebookManager.createNotebook(title, libraryModel.currentFolderId)
            libraryModel.refresh()
            nbTitleInput.text = ""
            root.notebookOpened(id)
        }
    }

    Dialog {
        id: newFolderDialog
        title: "Create New Folder"
        modal: true
        anchors.centerIn: parent
        width: 320
        standardButtons: Dialog.Ok | Dialog.Cancel

        contentItem: ColumnLayout {
            spacing: 10
            TextField {
                id: folderNameInput
                Layout.fillWidth: true
                placeholderText: "Folder Name (e.g., Fall Semester)"
                color: themeManager.onSurface
            }
        }
        onAccepted: {
            var name = folderNameInput.text.trim()
            if (name !== "") {
                notebookManager.createFolder(name, libraryModel.currentFolderId)
                libraryModel.refresh()
                folderNameInput.text = ""
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // 1. TOP HEADER & SEARCH
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Inkwell"
                font.bold: true
                font.pixelSize: 24
                color: themeManager.onSurface
            }

            Rectangle {
                width: 24
                height: 24
                radius: 6
                color: themeManager.primary
                Text {
                    anchors.centerIn: parent
                    text: "✒"
                    color: "#FFFFFF"
                    font.pixelSize: 13
                }
            }

            Item { Layout.fillWidth: true }

            // New Folder button
            Button {
                text: "+ Folder"
                onClicked: newFolderDialog.open()
            }

            // New Notebook button
            Button {
                text: "+ New Notebook"
                highlighted: true
                onClicked: newNotebookDialog.open()
            }

            // Theme toggle
            ToolButton {
                text: themeManager.isDark ? "☀️" : "🌙"
                font.pixelSize: 16
                ToolTip.text: "Toggle Theme"
                ToolTip.visible: hovered
                onClicked: themeManager.toggleTheme()
            }

            // Settings
            ToolButton {
                text: "⚙"
                font.pixelSize: 16
                ToolTip.text: "Settings"
                ToolTip.visible: hovered
                onClicked: settingsDialog.open()
            }
        }

        // Folder back breadcrumb if inside a subfolder
        if (libraryModel.currentFolderId !== "") {
            RowLayout {
                Button {
                    text: "← Back to Root Library"
                    onClicked: {
                        libraryModel.currentFolderId = ""
                        libraryModel.refresh()
                    }
                }
            }
        }

        // 2. NOTEBOOK & FOLDER GRID
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            GridView {
                id: grid
                anchors.fill: parent
                cellWidth: 200
                cellHeight: 250
                model: libraryModel

                delegate: Item {
                    width: grid.cellWidth
                    height: grid.cellHeight

                    Rectangle {
                        anchors.centerIn: parent
                        width: 175
                        height: 225
                        radius: 12
                        color: themeManager.surface
                        border.color: themeManager.outline
                        border.width: 1

                        // Hover elevation
                        scale: itemMouseArea.containsMouse ? 1.03 : 1.0
                        Behavior on scale { NumberAnimation { duration: 120 } }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 8

                            // Book Cover Visual
                            Rectangle {
                                Layout.fillWidth: true
                                height: 140
                                radius: 8
                                color: model.type === "folder" ? "#F59E0B" : (model.coverColor ? model.coverColor : "#3B82F6")

                                Text {
                                    anchors.centerIn: parent
                                    text: model.type === "folder" ? "📁" : "📖"
                                    font.pixelSize: 42
                                }
                            }

                            // Title
                            Text {
                                text: model.title
                                color: themeManager.onSurface
                                font.bold: true
                                font.pixelSize: 13
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            // Subtitle / Date
                            Text {
                                text: model.type === "folder" ? "Folder" : (model.modifiedAt ? model.modifiedAt.substring(0, 10) : "")
                                color: themeManager.onSurfaceVariant
                                font.pixelSize: 11
                            }
                        }

                        MouseArea {
                            id: itemMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            onClicked: function(mouse) {
                                if (mouse.button === Qt.LeftButton) {
                                    if (model.type === "folder") {
                                        libraryModel.currentFolderId = model.id
                                        libraryModel.refresh()
                                    } else {
                                        root.notebookOpened(model.id)
                                    }
                                } else if (mouse.button === Qt.RightButton) {
                                    contextMenu.popup()
                                }
                            }

                            Menu {
                                id: contextMenu
                                MenuItem {
                                    text: "Open"
                                    onTriggered: {
                                        if (model.type === "folder") {
                                            libraryModel.currentFolderId = model.id
                                            libraryModel.refresh()
                                        } else {
                                            root.notebookOpened(model.id)
                                        }
                                    }
                                }
                                MenuItem {
                                    text: "Delete"
                                    onTriggered: {
                                        if (model.type === "folder") {
                                            notebookManager.deleteFolder(model.id)
                                        } else {
                                            notebookManager.deleteNotebook(model.id)
                                        }
                                        libraryModel.refresh()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
