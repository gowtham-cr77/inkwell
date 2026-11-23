import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

import "pages"

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 800
    minimumWidth: 850
    minimumHeight: 600
    title: "Inkwell — Handwritten Notes"
    color: themeManager.background

    property string currentView: "library" // "library" or "editor"
    property string activeNotebookId: ""

    // Keyboard Shortcuts
    Shortcut {
        sequence: "Ctrl+Z"
        onActivated: if (currentView === "editor") editorPage.undo()
    }
    Shortcut {
        sequence: "Ctrl+Y"
        onActivated: if (currentView === "editor") editorPage.redo()
    }
    Shortcut {
        sequence: "Ctrl+S"
        onActivated: if (currentView === "editor") editorPage.save()
    }
    Shortcut {
        sequence: "Ctrl+N"
        onActivated: createNewNotebook()
    }
    Shortcut {
        sequence: "Ctrl+="
        onActivated: if (currentView === "editor") editorPage.zoomIn()
    }
    Shortcut {
        sequence: "Ctrl+-"
        onActivated: if (currentView === "editor") editorPage.zoomOut()
    }
    Shortcut {
        sequence: "F11"
        onActivated: toggleFullscreen()
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: libraryPage
    }

    LibraryPage {
        id: libraryPage
        visible: false
        onNotebookOpened: function(id) {
            window.activeNotebookId = id
            window.currentView = "editor"
            editorPage.notebookId = id
            stackView.push(editorPage)
        }
    }

    EditorPage {
        id: editorPage
        visible: false
        onBackToLibrary: {
            window.currentView = "library"
            stackView.pop()
            libraryPage.refresh()
        }
    }

    function createNewNotebook() {
        var id = notebookManager.createNotebook("Untitled Notebook")
        window.activeNotebookId = id
        window.currentView = "editor"
        editorPage.notebookId = id
        stackView.push(editorPage)
    }

    function toggleFullscreen() {
        if (window.visibility === Window.FullScreen) {
            window.showNormal()
        } else {
            window.showFullScreen()
        }
    }
}
