import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Inkwell 1.0

import "../components"
import "../dialogs"

Item {
    id: root

    property string notebookId: ""
    property string notebookTitle: "Untitled Notebook"

    signal backToLibrary()

    // Load notebook data whenever notebookId changes
    onNotebookIdChanged: {
        if (notebookId !== "") {
            var json = notebookManager.loadNotebook(notebookId)
            if (json !== "") {
                canvas.setNotebook(json)
                var parsed = JSON.parse(json)
                if (parsed.title) {
                    root.notebookTitle = parsed.title
                }
            }
        }
    }

    Connections {
        target: canvas
        function onPageModified() {
            var updatedJson = canvas.getNotebookJson()
            notebookManager.saveNotebook(updatedJson)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 1. TOP TOOLBAR
        Toolbar {
            id: toolbar
            Layout.fillWidth: true
            notebookTitle: root.notebookTitle
            activeTool: canvas.currentTool
            activePenColor: canvas.penColor
            activePenWidth: canvas.penWidth
            activeHighlighterColor: canvas.highlighterColor
            activeHighlighterWidth: canvas.highlighterWidth
            canUndo: canvas.canUndo
            canRedo: canvas.canRedo
            rulerActive: canvas.rulerVisible
            protractorActive: canvas.protractorVisible

            onBackClicked: {
                // Auto-save before leaving
                var updatedJson = canvas.getNotebookJson()
                notebookManager.saveNotebook(updatedJson)
                root.backToLibrary()
            }
            onToolSelected: function(t) { canvas.currentTool = t }
            onUndoClicked: canvas.undo()
            onRedoClicked: canvas.redo()
            onRulerToggled: canvas.rulerVisible = !canvas.rulerVisible
            onProtractorToggled: canvas.protractorVisible = !canvas.protractorVisible
            onImportPdfClicked: fileDialog.open()
            onExportPdfClicked: pdfDialog.open()
        }

        // 2. CENTRAL WORKSPACE
        Item {
            id: workspaceArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            InkCanvas {
                id: canvas
                anchors.fill: parent
                focus: true
            }

            // Bottom-Center Page Navigator
            PageNavigator {
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 18
                anchors.horizontalCenter: parent.horizontalCenter
                currentPage: canvas.currentPageIndex
                totalPages: canvas.pageCount

                onPrevClicked: canvas.previousPage()
                onNextClicked: canvas.nextPage()
                onAddClicked: canvas.addPage()
                onDuplicateClicked: canvas.duplicateCurrentPage()
                onDeleteClicked: canvas.deletePage(canvas.currentPageIndex)
            }

            // Bottom-Right Zoom Controls
            ZoomControls {
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 18
                anchors.right: parent.right
                anchors.rightMargin: 18
                zoomLevel: canvas.zoomLevel

                onZoomInClicked: canvas.zoomIn()
                onZoomOutClicked: canvas.zoomOut()
                onZoomFitClicked: canvas.zoomFit()
            }
        }
    }

    // PDF Export Dialog
    PdfExportDialog {
        id: pdfDialog
        anchors.centerIn: parent
        onAcceptedExport: function(path, withBg) {
            canvas.exportPdf(path, withBg)
        }
    }

    // PDF Import File Dialog
    FileDialog {
        id: fileDialog
        title: "Import PDF Document"
        nameFilters: ["PDF Files (*.pdf)"]
        onAccepted: {
            canvas.importPdf(selectedFile.toString())
        }
    }

    // Export public methods for shortcuts
    function undo() { canvas.undo() }
    function redo() { canvas.redo() }
    function zoomIn() { canvas.zoomIn() }
    function zoomOut() { canvas.zoomOut() }
    function save() {
        var updatedJson = canvas.getNotebookJson()
        notebookManager.saveNotebook(updatedJson)
    }
}
