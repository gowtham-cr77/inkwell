#pragma once

#include "../core/Types.h"
#include "../core/Stroke.h"
#include "../core/Page.h"
#include "../core/Notebook.h"
#include "../core/UndoCommands.h"
#include "InputHandler.h"
#include "StrokeRenderer.h"

#include <QQuickPaintedItem>
#include <QPainter>
#include <QUndoStack>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>
#include <vector>

namespace Inkwell {

class InkCanvas : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT

    // Current tool
    Q_PROPERTY(int currentTool READ currentToolInt WRITE setCurrentToolInt NOTIFY currentToolChanged)

    // Pen properties
    Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor NOTIFY penColorChanged)
    Q_PROPERTY(double penWidth READ penWidth WRITE setPenWidth NOTIFY penWidthChanged)
    Q_PROPERTY(double penOpacity READ penOpacity WRITE setPenOpacity NOTIFY penOpacityChanged)

    // Highlighter properties
    Q_PROPERTY(QColor highlighterColor READ highlighterColor WRITE setHighlighterColor NOTIFY highlighterColorChanged)
    Q_PROPERTY(double highlighterWidth READ highlighterWidth WRITE setHighlighterWidth NOTIFY highlighterWidthChanged)

    // Eraser properties
    Q_PROPERTY(double eraserWidth READ eraserWidth WRITE setEraserWidth NOTIFY eraserWidthChanged)
    Q_PROPERTY(int eraserMode READ eraserModeInt WRITE setEraserModeInt NOTIFY eraserModeChanged)

    // Shape properties
    Q_PROPERTY(int shapeType READ shapeTypeInt WRITE setShapeTypeInt NOTIFY shapeTypeChanged)
    Q_PROPERTY(bool shapeFilled READ shapeFilled WRITE setShapeFilled NOTIFY shapeFilledChanged)
    Q_PROPERTY(bool snapAngles READ snapAngles WRITE setSnapAngles NOTIFY snapAnglesChanged)

    // Laser pointer properties
    Q_PROPERTY(QColor laserColor READ laserColor WRITE setLaserColor NOTIFY laserColorChanged)
    Q_PROPERTY(double laserWidth READ laserWidth WRITE setLaserWidth NOTIFY laserWidthChanged)
    Q_PROPERTY(int laserMode READ laserMode WRITE setLaserMode NOTIFY laserModeChanged) // 0=Dot, 1=Line

    // Lasso selection properties
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectionChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(bool selectPenStrokes READ selectPenStrokes WRITE setSelectPenStrokes NOTIFY lassoFiltersChanged)
    Q_PROPERTY(bool selectHighlighterStrokes READ selectHighlighterStrokes WRITE setSelectHighlighterStrokes NOTIFY lassoFiltersChanged)
    Q_PROPERTY(bool selectShapes READ selectShapes WRITE setSelectShapes NOTIFY lassoFiltersChanged)

    // Ruler & Protractor tools
    Q_PROPERTY(bool rulerVisible READ rulerVisible WRITE setRulerVisible NOTIFY rulerVisibleChanged)
    Q_PROPERTY(bool protractorVisible READ protractorVisible WRITE setProtractorVisible NOTIFY protractorVisibleChanged)

    // View
    Q_PROPERTY(double zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(QPointF panOffset READ panOffset WRITE setPanOffset NOTIFY panOffsetChanged)

    // Page info
    Q_PROPERTY(int currentPageIndex READ currentPageIndex WRITE setCurrentPageIndex NOTIFY currentPageIndexChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    Q_PROPERTY(double pageWidth READ pageWidth NOTIFY pageDimensionsChanged)
    Q_PROPERTY(double pageHeight READ pageHeight NOTIFY pageDimensionsChanged)

    // Undo/Redo
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY undoStateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY undoStateChanged)

    // Paper
    Q_PROPERTY(int paperType READ paperTypeInt WRITE setPaperTypeInt NOTIFY paperTypeChanged)
    Q_PROPERTY(QColor paperColor READ paperColor WRITE setPaperColor NOTIFY paperColorChanged)
    Q_PROPERTY(double paperLineSpacing READ paperLineSpacing WRITE setPaperLineSpacing NOTIFY paperConfigChanged)
    Q_PROPERTY(double paperGridSize READ paperGridSize WRITE setPaperGridSize NOTIFY paperConfigChanged)

public:
    explicit InkCanvas(QQuickItem *parent = nullptr);
    ~InkCanvas() override;

    void paint(QPainter *painter) override;

    // Tool
    ToolType currentTool() const { return m_currentTool; }
    int currentToolInt() const { return static_cast<int>(m_currentTool); }
    void setCurrentTool(ToolType tool);
    void setCurrentToolInt(int tool) { setCurrentTool(static_cast<ToolType>(tool)); }

    // Pen properties
    QColor penColor() const { return m_penColor; }
    void setPenColor(const QColor &color);
    double penWidth() const { return m_penWidth; }
    void setPenWidth(double width);
    double penOpacity() const { return m_penOpacity; }
    void setPenOpacity(double opacity);

    // Highlighter properties
    QColor highlighterColor() const { return m_highlighterColor; }
    void setHighlighterColor(const QColor &color);
    double highlighterWidth() const { return m_highlighterWidth; }
    void setHighlighterWidth(double width);

    // Eraser
    double eraserWidth() const { return m_eraserWidth; }
    void setEraserWidth(double width);
    EraserMode eraserMode() const { return m_eraserMode; }
    int eraserModeInt() const { return static_cast<int>(m_eraserMode); }
    void setEraserModeInt(int mode);

    // Shapes
    ShapeType shapeType() const { return m_shapeType; }
    int shapeTypeInt() const { return static_cast<int>(m_shapeType); }
    void setShapeTypeInt(int type);
    bool shapeFilled() const { return m_shapeFilled; }
    void setShapeFilled(bool filled);
    bool snapAngles() const { return m_snapAngles; }
    void setSnapAngles(bool snap);

    // Laser
    QColor laserColor() const { return m_laserColor; }
    void setLaserColor(const QColor &color);
    double laserWidth() const { return m_laserWidth; }
    void setLaserWidth(double width);
    int laserMode() const { return m_laserMode; }
    void setLaserMode(int mode);

    // Lasso
    int selectedCount() const { return static_cast<int>(m_selectedStrokeIds.size()); }
    bool hasSelection() const { return !m_selectedStrokeIds.empty(); }
    bool selectPenStrokes() const { return m_selectPenStrokes; }
    void setSelectPenStrokes(bool val);
    bool selectHighlighterStrokes() const { return m_selectHighlighterStrokes; }
    void setSelectHighlighterStrokes(bool val);
    bool selectShapes() const { return m_selectShapes; }
    void setSelectShapes(bool val);

    // Ruler & Protractor
    bool rulerVisible() const { return m_rulerVisible; }
    void setRulerVisible(bool visible);
    bool protractorVisible() const { return m_protractorVisible; }
    void setProtractorVisible(bool visible);

    // View
    double zoomLevel() const { return m_zoomLevel; }
    void setZoomLevel(double zoom);
    QPointF panOffset() const { return m_panOffset; }
    void setPanOffset(const QPointF &offset);

    // Page
    int currentPageIndex() const { return m_currentPageIndex; }
    void setCurrentPageIndex(int index);
    int pageCount() const;
    double pageWidth() const;
    double pageHeight() const;

    // Paper
    int paperTypeInt() const;
    void setPaperTypeInt(int type);
    QColor paperColor() const;
    void setPaperColor(const QColor &color);
    double paperLineSpacing() const;
    void setPaperLineSpacing(double spacing);
    double paperGridSize() const;
    void setPaperGridSize(double size);

    // Undo/Redo
    bool canUndo() const { return m_undoStack.canUndo(); }
    bool canRedo() const { return m_undoStack.canRedo(); }

    // Notebook
    Q_INVOKABLE void setNotebook(const QString &notebookJson);
    Q_INVOKABLE QString getNotebookJson() const;
    Q_INVOKABLE void newNotebook(const QString &title);

    // Page operations
    Q_INVOKABLE void addPage();
    Q_INVOKABLE void deletePage(int index);
    Q_INVOKABLE void nextPage();
    Q_INVOKABLE void previousPage();
    Q_INVOKABLE void duplicateCurrentPage();
    Q_INVOKABLE void movePage(int from, int to);

    // Actions
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void clearPage();
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    Q_INVOKABLE void zoomFit();

    // Lasso actions
    Q_INVOKABLE void deleteSelected();
    Q_INVOKABLE void recolorSelected(const QColor &color);
    Q_INVOKABLE void copySelected();
    Q_INVOKABLE void pasteSelected();
    Q_INVOKABLE void clearSelection();

    // PDF Import & Export
    Q_INVOKABLE bool importPdf(const QString &filePath);
    Q_INVOKABLE bool exportPdf(const QString &filePath, bool includeBackground = true);
    Q_INVOKABLE bool exportPageImage(const QString &filePath, int pageIndex = -1);

    // OCR
    Q_INVOKABLE QString performOcrOnSelection();
    Q_INVOKABLE QString performOcrOnPage();

signals:
    void currentToolChanged();
    void penColorChanged();
    void penWidthChanged();
    void penOpacityChanged();
    void highlighterColorChanged();
    void highlighterWidthChanged();
    void eraserWidthChanged();
    void eraserModeChanged();
    void shapeTypeChanged();
    void shapeFilledChanged();
    void snapAnglesChanged();
    void laserColorChanged();
    void laserWidthChanged();
    void laserModeChanged();
    void selectionChanged();
    void lassoFiltersChanged();
    void rulerVisibleChanged();
    void protractorVisibleChanged();
    void zoomLevelChanged();
    void panOffsetChanged();
    void currentPageIndexChanged();
    void pageCountChanged();
    void pageDimensionsChanged();
    void undoStateChanged();
    void paperTypeChanged();
    void paperColorChanged();
    void paperConfigChanged();
    void notebookChanged();
    void pageModified();

protected:
    void tabletEvent(QTabletEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;

private slots:
    void onStrokeStarted(const InputPoint &point);
    void onStrokeUpdated(const InputPoint &point);
    void onStrokeFinished(const InputPoint &point);
    void onAutoSave();
    void onLaserFadeTick();

private:
    Page* currentPage();
    const Page* currentPage() const;
    QPointF mapToPage(const QPointF &windowPos) const;
    QPointF mapFromPage(const QPointF &pagePos) const;
    void handleErase(const QPointF &pagePos);
    void handleLassoSelect(const QPainterPath &lassoPath);
    void finishShapeStroke(const QPointF &startPt, const QPointF &endPt);

    // Tool state
    ToolType m_currentTool = ToolType::Pen;
    QColor m_penColor = QColor(0, 0, 0);
    double m_penWidth = 2.5;
    double m_penOpacity = 1.0;
    QColor m_highlighterColor = QColor(255, 235, 59);
    double m_highlighterWidth = 16.0;
    double m_eraserWidth = 22.0;
    EraserMode m_eraserMode = EraserMode::StrokeEraser;

    // Shapes
    ShapeType m_shapeType = ShapeType::Line;
    bool m_shapeFilled = false;
    bool m_snapAngles = false;
    QPointF m_shapeStartPos;

    // Laser
    QColor m_laserColor = QColor(255, 30, 30);
    double m_laserWidth = 6.0;
    int m_laserMode = 1; // 0=Dot, 1=Line
    std::vector<InputPoint> m_laserPoints;
    double m_laserOpacity = 1.0;
    QTimer m_laserTimer;

    // Lasso
    std::vector<QString> m_selectedStrokeIds;
    std::vector<Stroke> m_clipboardStrokes;
    QPainterPath m_activeLassoPath;
    bool m_isMovingSelection = false;
    QPointF m_selectionDragStart;
    bool m_selectPenStrokes = true;
    bool m_selectHighlighterStrokes = true;
    bool m_selectShapes = true;

    // Ruler & Protractor
    bool m_rulerVisible = false;
    bool m_protractorVisible = false;
    QPointF m_rulerPos = QPointF(100, 200);
    double m_rulerAngle = 0.0;
    QPointF m_protractorPos = QPointF(200, 200);

    // View state
    double m_zoomLevel = 1.0;
    QPointF m_panOffset = QPointF(40, 40);
    bool m_isPanning = false;
    QPointF m_lastPanPos;

    // Notebook state
    std::unique_ptr<Notebook> m_notebook;
    int m_currentPageIndex = 0;

    // Drawing state
    InputHandler m_inputHandler;
    StrokeRenderer m_strokeRenderer;
    Stroke m_activeStroke;
    bool m_hasActiveStroke = false;
    bool m_tabletActive = false;

    // Undo
    QUndoStack m_undoStack;

    // Auto-save timer
    QTimer m_autoSaveTimer;
    bool m_dirty = false;
};

} // namespace Inkwell
