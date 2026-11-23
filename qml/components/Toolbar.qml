import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    height: 54
    color: themeManager.surface
    border.color: themeManager.outline
    border.width: 1

    property int activeTool: 0 // 0=Pen, 1=Highlighter, 2=Eraser, 3=Laser, 4=Lasso, 5=Shape
    property color activePenColor: "#000000"
    property real activePenWidth: 2.5
    property color activeHighlighterColor: "#FEF08A"
    property real activeHighlighterWidth: 16.0
    property string notebookTitle: "Notebook"

    property bool canUndo: false
    property bool canRedo: false
    property bool rulerActive: false
    property bool protractorActive: false

    signal toolSelected(int tool)
    signal backClicked()
    signal undoClicked()
    signal redoClicked()
    signal exportPdfClicked()
    signal importPdfClicked()
    signal rulerToggled()
    signal protractorToggled()

    // Tool popups
    PenSettingsPopup {
        id: penPopup
        isHighlighter: false
        activeColor: root.activePenColor
        activeWidth: root.activePenWidth
        onColorChanged: function(c) {
            root.activePenColor = c
            canvas.penColor = c
        }
        onWidthChanged: function(w) {
            root.activePenWidth = w
            canvas.penWidth = w
        }
    }

    PenSettingsPopup {
        id: highlighterPopup
        isHighlighter: true
        activeColor: root.activeHighlighterColor
        activeWidth: root.activeHighlighterWidth
        onColorChanged: function(c) {
            root.activeHighlighterColor = c
            canvas.highlighterColor = c
        }
        onWidthChanged: function(w) {
            root.activeHighlighterWidth = w
            canvas.highlighterWidth = w
        }
    }

    ShapePickerPopup {
        id: shapePopup
        activeShapeType: canvas.shapeType
        activeFilled: canvas.shapeFilled
        activeSnapAngles: canvas.snapAngles
        onShapeSelected: function(t) { canvas.shapeType = t }
        onFilledToggled: function(f) { canvas.shapeFilled = f }
        onSnapAnglesToggled: function(s) { canvas.snapAngles = s }
    }

    LaserSettingsPopup {
        id: laserPopup
        activeLaserColor: canvas.laserColor
        activeLaserWidth: canvas.laserWidth
        activeLaserMode: canvas.laserMode
        onColorChanged: function(c) { canvas.laserColor = c }
        onWidthChanged: function(w) { canvas.laserWidth = w }
        onModeChanged: function(m) { canvas.laserMode = m }
    }

    LassoSettingsPopup {
        id: lassoPopup
        selectPen: canvas.selectPenStrokes
        selectHighlighter: canvas.selectHighlighterStrokes
        selectShapes: canvas.selectShapes
        selectedCount: canvas.selectedCount
        onTogglePen: function(v) { canvas.selectPenStrokes = v }
        onToggleHighlighter: function(v) { canvas.selectHighlighterStrokes = v }
        onToggleShapes: function(v) { canvas.selectShapes = v }
        onDeleteTriggered: canvas.deleteSelected()
        onCopyTriggered: canvas.copySelected()
        onPasteTriggered: canvas.pasteSelected()
        onOcrTriggered: {
            var text = canvas.performOcrOnSelection()
            ocrDialog.recognizedText = text
            ocrDialog.open()
        }
        onRecolorTriggered: function(c) { canvas.recolorSelected(c) }
    }

    PaperSettingsPopup {
        id: paperPopup
        activePaperType: canvas.paperType
        activePaperColor: canvas.paperColor
        activeLineSpacing: canvas.paperLineSpacing
        onPaperTypeSelected: function(t) { canvas.paperType = t }
        onPaperColorSelected: function(c) { canvas.paperColor = c }
        onLineSpacingChanged: function(s) { canvas.paperLineSpacing = s }
    }

    Dialog {
        id: ocrDialog
        title: "Recognized Handwriting Text"
        modal: true
        anchors.centerIn: parent
        width: 360
        standardButtons: Dialog.Ok
        property string recognizedText: ""

        contentItem: TextArea {
            text: ocrDialog.recognizedText
            wrapMode: Text.WordWrap
            readOnly: false
            font.pixelSize: 13
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 4

        // Back to Library
        ToolButton {
            text: "← Library"
            font.bold: true
            onClicked: root.backClicked()
        }

        Text {
            text: root.notebookTitle
            color: themeManager.onSurface
            font.bold: true
            font.pixelSize: 13
            Layout.preferredWidth: 140
            elide: Text.ElideRight
        }

        ToolSeparator {
            contentItem: Rectangle { implicitWidth: 1; color: themeManager.outline }
        }

        // TOOL GROUP (Noteastic-style toolbar: tap to select, tap again for flyout settings)
        // 1. PEN
        ToolButton {
            id: penBtn
            text: "🖊 Pen"
            font.bold: true
            highlighted: root.activeTool === 0
            onClicked: {
                if (root.activeTool === 0) {
                    penPopup.x = penBtn.x
                    penPopup.y = penBtn.y + penBtn.height + 4
                    penPopup.open()
                } else {
                    root.toolSelected(0)
                }
            }
        }

        // 2. HIGHLIGHTER
        ToolButton {
            id: hlBtn
            text: "🖍 Marker"
            font.bold: true
            highlighted: root.activeTool === 1
            onClicked: {
                if (root.activeTool === 1) {
                    highlighterPopup.x = hlBtn.x
                    highlighterPopup.y = hlBtn.y + hlBtn.height + 4
                    highlighterPopup.open()
                } else {
                    root.toolSelected(1)
                }
            }
        }

        // 3. ERASER
        ToolButton {
            text: "⌫ Eraser"
            font.bold: true
            highlighted: root.activeTool === 2
            onClicked: root.toolSelected(2)
        }

        // 4. LASSO
        ToolButton {
            id: lassoBtn
            text: "✂ Lasso"
            font.bold: true
            highlighted: root.activeTool === 4
            onClicked: {
                if (root.activeTool === 4) {
                    lassoPopup.x = lassoBtn.x
                    lassoPopup.y = lassoBtn.y + lassoBtn.height + 4
                    lassoPopup.open()
                } else {
                    root.toolSelected(4)
                }
            }
        }

        // 5. SHAPES
        ToolButton {
            id: shapeBtn
            text: "△ Shapes"
            font.bold: true
            highlighted: root.activeTool === 5
            onClicked: {
                if (root.activeTool === 5) {
                    shapePopup.x = shapeBtn.x
                    shapePopup.y = shapeBtn.y + shapeBtn.height + 4
                    shapePopup.open()
                } else {
                    root.toolSelected(5)
                }
            }
        }

        // 6. LASER POINTER
        ToolButton {
            id: laserBtn
            text: "🔴 Laser"
            font.bold: true
            highlighted: root.activeTool === 3
            onClicked: {
                if (root.activeTool === 3) {
                    laserPopup.x = laserBtn.x
                    laserPopup.y = laserBtn.y + laserBtn.height + 4
                    laserPopup.open()
                } else {
                    root.toolSelected(3)
                }
            }
        }

        ToolSeparator {
            contentItem: Rectangle { implicitWidth: 1; color: themeManager.outline }
        }

        // Precision Instruments: Ruler & Protractor
        ToolButton {
            text: "📏 Ruler"
            checkable: true
            checked: root.rulerActive
            onClicked: root.rulerToggled()
        }

        ToolButton {
            text: "📐 Protractor"
            checkable: true
            checked: root.protractorActive
            onClicked: root.protractorToggled()
        }

        // Paper background settings
        ToolButton {
            id: paperBtn
            text: "📑 Paper"
            onClicked: {
                paperPopup.x = paperBtn.x
                paperPopup.y = paperBtn.y + paperBtn.height + 4
                paperPopup.open()
            }
        }

        Item { Layout.fillWidth: true }

        // Action controls
        ToolButton {
            text: "↶"
            font.pixelSize: 18
            enabled: root.canUndo
            ToolTip.text: "Undo (Ctrl+Z)"
            ToolTip.visible: hovered
            onClicked: root.undoClicked()
        }

        ToolButton {
            text: "↷"
            font.pixelSize: 18
            enabled: root.canRedo
            ToolTip.text: "Redo (Ctrl+Y)"
            ToolTip.visible: hovered
            onClicked: root.redoClicked()
        }

        ToolSeparator {
            contentItem: Rectangle { implicitWidth: 1; color: themeManager.outline }
        }

        // PDF Actions
        ToolButton {
            text: "PDF ⤓"
            ToolTip.text: "Import PDF Document"
            ToolTip.visible: hovered
            onClicked: root.importPdfClicked()
        }

        ToolButton {
            text: "PDF ⤒"
            ToolTip.text: "Export as PDF"
            ToolTip.visible: hovered
            onClicked: root.exportPdfClicked()
        }
    }
}
