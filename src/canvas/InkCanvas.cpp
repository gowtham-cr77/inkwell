#include "InkCanvas.h"
#include <QPainter>
#include <QTabletEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QHoverEvent>
#include <QPdfWriter>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <cmath>

namespace Inkwell {

InkCanvas::InkCanvas(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
    setAcceptTouchEvents(true);
    setAcceptHoverEvents(true);
    setAntialiasing(true);
    setRenderTarget(QQuickPaintedItem::FramebufferObject);

    // InputHandler connection
    connect(&m_inputHandler, &InputHandler::strokeStarted, this, &InkCanvas::onStrokeStarted);
    connect(&m_inputHandler, &InputHandler::strokeUpdated, this, &InkCanvas::onStrokeUpdated);
    connect(&m_inputHandler, &InputHandler::strokeFinished, this, &InkCanvas::onStrokeFinished);

    // Laser fading timer
    connect(&m_laserTimer, &QTimer::timeout, this, &InkCanvas::onLaserFadeTick);

    // Undo stack connections
    connect(&m_undoStack, &QUndoStack::canUndoChanged, this, &InkCanvas::undoStateChanged);
    connect(&m_undoStack, &QUndoStack::canRedoChanged, this, &InkCanvas::undoStateChanged);

    // Auto-save timer (every 10 seconds)
    connect(&m_autoSaveTimer, &QTimer::timeout, this, &InkCanvas::onAutoSave);
    m_autoSaveTimer.start(10000);

    // Create initial notebook with one blank page
    m_notebook = std::make_unique<Notebook>("Quick Notes");
    m_notebook->addPage(Page(PageSize::A4));
}

InkCanvas::~InkCanvas()
{
}

Page* InkCanvas::currentPage()
{
    if (!m_notebook) return nullptr;
    return m_notebook->page(m_currentPageIndex);
}

const Page* InkCanvas::currentPage() const
{
    if (!m_notebook) return nullptr;
    return m_notebook->page(m_currentPageIndex);
}

int InkCanvas::pageCount() const
{
    return m_notebook ? m_notebook->pageCount() : 0;
}

double InkCanvas::pageWidth() const
{
    const Page *p = currentPage();
    return p ? p->width() : 595.0;
}

double InkCanvas::pageHeight() const
{
    const Page *p = currentPage();
    return p ? p->height() : 842.0;
}

int InkCanvas::paperTypeInt() const
{
    const Page *p = currentPage();
    return p ? static_cast<int>(p->paperConfig().type) : 0;
}

void InkCanvas::setPaperTypeInt(int type)
{
    Page *p = currentPage();
    if (p) {
        PaperConfig cfg = p->paperConfig();
        cfg.type = static_cast<PaperType>(type);
        p->setPaperConfig(cfg);
        emit paperTypeChanged();
        update();
    }
}

QColor InkCanvas::paperColor() const
{
    const Page *p = currentPage();
    return p ? p->paperConfig().backgroundColor : QColor(Qt::white);
}

void InkCanvas::setPaperColor(const QColor &color)
{
    Page *p = currentPage();
    if (p) {
        PaperConfig cfg = p->paperConfig();
        cfg.backgroundColor = color;
        p->setPaperConfig(cfg);
        emit paperColorChanged();
        update();
    }
}

double InkCanvas::paperLineSpacing() const
{
    const Page *p = currentPage();
    return p ? p->paperConfig().lineSpacing : 30.0;
}

void InkCanvas::setPaperLineSpacing(double spacing)
{
    Page *p = currentPage();
    if (p) {
        PaperConfig cfg = p->paperConfig();
        cfg.lineSpacing = spacing;
        p->setPaperConfig(cfg);
        emit paperConfigChanged();
        update();
    }
}

double InkCanvas::paperGridSize() const
{
    const Page *p = currentPage();
    return p ? p->paperConfig().gridSize : 30.0;
}

void InkCanvas::setPaperGridSize(double size)
{
    Page *p = currentPage();
    if (p) {
        PaperConfig cfg = p->paperConfig();
        cfg.gridSize = size;
        p->setPaperConfig(cfg);
        emit paperConfigChanged();
        update();
    }
}

void InkCanvas::setCurrentTool(ToolType tool)
{
    if (m_currentTool != tool) {
        m_currentTool = tool;
        if (tool != ToolType::Lasso) {
            clearSelection();
        }
        emit currentToolChanged();
        update();
    }
}

void InkCanvas::setPenColor(const QColor &color)
{
    if (m_penColor != color) {
        m_penColor = color;
        emit penColorChanged();
    }
}

void InkCanvas::setPenWidth(double width)
{
    if (std::abs(m_penWidth - width) > 0.001) {
        m_penWidth = width;
        emit penWidthChanged();
    }
}

void InkCanvas::setPenOpacity(double opacity)
{
    if (std::abs(m_penOpacity - opacity) > 0.001) {
        m_penOpacity = opacity;
        emit penOpacityChanged();
    }
}

void InkCanvas::setHighlighterColor(const QColor &color)
{
    if (m_highlighterColor != color) {
        m_highlighterColor = color;
        emit highlighterColorChanged();
    }
}

void InkCanvas::setHighlighterWidth(double width)
{
    if (std::abs(m_highlighterWidth - width) > 0.001) {
        m_highlighterWidth = width;
        emit highlighterWidthChanged();
    }
}

void InkCanvas::setEraserWidth(double width)
{
    if (std::abs(m_eraserWidth - width) > 0.001) {
        m_eraserWidth = width;
        emit eraserWidthChanged();
    }
}

void InkCanvas::setEraserModeInt(int mode)
{
    m_eraserMode = static_cast<EraserMode>(mode);
    emit eraserModeChanged();
}

void InkCanvas::setShapeTypeInt(int type)
{
    m_shapeType = static_cast<ShapeType>(type);
    emit shapeTypeChanged();
}

void InkCanvas::setShapeFilled(bool filled)
{
    if (m_shapeFilled != filled) {
        m_shapeFilled = filled;
        emit shapeFilledChanged();
    }
}

void InkCanvas::setSnapAngles(bool snap)
{
    if (m_snapAngles != snap) {
        m_snapAngles = snap;
        emit snapAnglesChanged();
    }
}

void InkCanvas::setLaserColor(const QColor &color)
{
    if (m_laserColor != color) {
        m_laserColor = color;
        emit laserColorChanged();
    }
}

void InkCanvas::setLaserWidth(double width)
{
    if (std::abs(m_laserWidth - width) > 0.001) {
        m_laserWidth = width;
        emit laserWidthChanged();
    }
}

void InkCanvas::setLaserMode(int mode)
{
    if (m_laserMode != mode) {
        m_laserMode = mode;
        emit laserModeChanged();
    }
}

void InkCanvas::setSelectPenStrokes(bool val)
{
    m_selectPenStrokes = val;
    emit lassoFiltersChanged();
}

void InkCanvas::setSelectHighlighterStrokes(bool val)
{
    m_selectHighlighterStrokes = val;
    emit lassoFiltersChanged();
}

void InkCanvas::setSelectShapes(bool val)
{
    m_selectShapes = val;
    emit lassoFiltersChanged();
}

void InkCanvas::setRulerVisible(bool visible)
{
    if (m_rulerVisible != visible) {
        m_rulerVisible = visible;
        emit rulerVisibleChanged();
        update();
    }
}

void InkCanvas::setProtractorVisible(bool visible)
{
    if (m_protractorVisible != visible) {
        m_protractorVisible = visible;
        emit protractorVisibleChanged();
        update();
    }
}

void InkCanvas::setZoomLevel(double zoom)
{
    double clamped = std::max(0.1, std::min(6.0, zoom));
    if (std::abs(m_zoomLevel - clamped) > 0.001) {
        m_zoomLevel = clamped;
        emit zoomLevelChanged();
        update();
    }
}

void InkCanvas::setPanOffset(const QPointF &offset)
{
    if (m_panOffset != offset) {
        m_panOffset = offset;
        emit panOffsetChanged();
        update();
    }
}

void InkCanvas::setCurrentPageIndex(int index)
{
    if (!m_notebook) return;
    int clamped = std::max(0, std::min(m_notebook->pageCount() - 1, index));
    if (m_currentPageIndex != clamped) {
        m_currentPageIndex = clamped;
        clearSelection();
        emit currentPageIndexChanged();
        emit pageDimensionsChanged();
        emit paperTypeChanged();
        emit paperColorChanged();
        update();
    }
}

QPointF InkCanvas::mapToPage(const QPointF &windowPos) const
{
    return (windowPos - m_panOffset) / m_zoomLevel;
}

QPointF InkCanvas::mapFromPage(const QPointF &pagePos) const
{
    return pagePos * m_zoomLevel + m_panOffset;
}

// Tablet and pointer events
void InkCanvas::tabletEvent(QTabletEvent *event)
{
    m_tabletActive = true;
    QPointF pagePos = mapToPage(event->position());
    double pressure = std::max(0.01, std::min(1.0, event->pressure()));

    switch (event->type()) {
    case QEvent::TabletPress:
        m_inputHandler.handlePenDown(pagePos, pressure, event->timestamp());
        event->accept();
        break;
    case QEvent::TabletMove:
        m_inputHandler.handlePenMove(pagePos, pressure, event->timestamp());
        event->accept();
        break;
    case QEvent::TabletRelease:
        m_inputHandler.handlePenUp(pagePos, pressure, event->timestamp());
        m_tabletActive = false;
        event->accept();
        break;
    default:
        event->ignore();
        break;
    }
}

void InkCanvas::mousePressEvent(QMouseEvent *event)
{
    if (m_tabletActive) {
        event->accept();
        return;
    }

    if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && (event->modifiers() & Qt::AltModifier))) {
        m_isPanning = true;
        m_lastPanPos = event->position();
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        QPointF pagePos = mapToPage(event->position());

        if (m_currentTool == ToolType::Lasso && hasSelection()) {
            // Check if clicking inside selection bounding box to start move
            QRectF selBox;
            Page *p = currentPage();
            if (p) {
                for (const auto &id : m_selectedStrokeIds) {
                    Stroke *s = p->strokeById(id);
                    if (s) selBox = selBox.united(s->boundingRect());
                }
            }
            if (selBox.contains(pagePos)) {
                m_isMovingSelection = true;
                m_selectionDragStart = pagePos;
                event->accept();
                return;
            }
        }

        m_inputHandler.handleMouseDown(pagePos, event->timestamp());
        event->accept();
    }
}

void InkCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (m_tabletActive) {
        event->accept();
        return;
    }

    if (m_isPanning) {
        QPointF delta = event->position() - m_lastPanPos;
        m_panOffset += delta;
        m_lastPanPos = event->position();
        emit panOffsetChanged();
        update();
        event->accept();
        return;
    }

    if (m_isMovingSelection) {
        QPointF pagePos = mapToPage(event->position());
        QPointF delta = pagePos - m_selectionDragStart;
        m_selectionDragStart = pagePos;

        Page *p = currentPage();
        if (p) {
            for (const auto &id : m_selectedStrokeIds) {
                Stroke *s = p->strokeById(id);
                if (s) s->translate(delta);
            }
            update();
        }
        event->accept();
        return;
    }

    if (event->buttons() & Qt::LeftButton) {
        QPointF pagePos = mapToPage(event->position());
        m_inputHandler.handleMouseMove(pagePos, event->timestamp());
        event->accept();
    }
}

void InkCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_tabletActive) {
        event->accept();
        return;
    }

    if (m_isPanning && (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)) {
        m_isPanning = false;
        event->accept();
        return;
    }

    if (m_isMovingSelection && event->button() == Qt::LeftButton) {
        m_isMovingSelection = false;
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        QPointF pagePos = mapToPage(event->position());
        m_inputHandler.handleMouseUp(pagePos, event->timestamp());
        event->accept();
    }
}

void InkCanvas::wheelEvent(QWheelEvent *event)
{
    QPointF cursorWindowPos = event->position();
    QPointF cursorPageBefore = mapToPage(cursorWindowPos);

    double factor = event->angleDelta().y() > 0 ? 1.15 : 0.85;
    double newZoom = std::max(0.1, std::min(6.0, m_zoomLevel * factor));

    m_zoomLevel = newZoom;
    m_panOffset = cursorWindowPos - cursorPageBefore * m_zoomLevel;

    emit zoomLevelChanged();
    emit panOffsetChanged();
    update();
    event->accept();
}

void InkCanvas::hoverMoveEvent(QHoverEvent *event)
{
    if (m_currentTool == ToolType::LaserPointer) {
        QPointF pagePos = mapToPage(event->position());
        InputPoint pt;
        pt.x = pagePos.x();
        pt.y = pagePos.y();
        pt.pressure = 1.0;
        pt.timestamp = QDateTime::currentMSecsSinceEpoch();

        m_laserPoints.push_back(pt);
        if (m_laserPoints.size() > 25) {
            m_laserPoints.erase(m_laserPoints.begin());
        }
        m_laserOpacity = 1.0;
        if (!m_laserTimer.isActive()) {
            m_laserTimer.start(30);
        }
        update();
    }
}

void InkCanvas::onLaserFadeTick()
{
    m_laserOpacity -= 0.05;
    if (m_laserOpacity <= 0.0) {
        m_laserOpacity = 0.0;
        m_laserPoints.clear();
        m_laserTimer.stop();
    }
    update();
}

// Drawing slots
void InkCanvas::onStrokeStarted(const InputPoint &point)
{
    if (m_currentTool == ToolType::Eraser) {
        handleErase(point.toPointF());
        return;
    }

    if (m_currentTool == ToolType::Lasso) {
        m_activeLassoPath = QPainterPath();
        m_activeLassoPath.moveTo(point.toPointF());
        m_hasActiveStroke = true;
        update();
        return;
    }

    if (m_currentTool == ToolType::LaserPointer) {
        m_laserPoints.clear();
        m_laserPoints.push_back(point);
        m_laserOpacity = 1.0;
        m_laserTimer.start(30);
        m_hasActiveStroke = true;
        update();
        return;
    }

    if (m_currentTool == ToolType::Shape) {
        m_shapeStartPos = point.toPointF();
        m_activeStroke = Stroke(ToolType::Pen);
        m_activeStroke.setColor(m_penColor);
        m_activeStroke.setBaseWidth(m_penWidth);
        m_activeStroke.setOpacity(m_penOpacity);
        m_activeStroke.addPoint(point);
        m_hasActiveStroke = true;
        update();
        return;
    }

    // Pen or Highlighter
    m_activeStroke = Stroke(m_currentTool);
    if (m_currentTool == ToolType::Highlighter) {
        m_activeStroke.setColor(m_highlighterColor);
        m_activeStroke.setBaseWidth(m_highlighterWidth);
        m_activeStroke.setOpacity(0.5);
    } else {
        m_activeStroke.setColor(m_penColor);
        m_activeStroke.setBaseWidth(m_penWidth);
        m_activeStroke.setOpacity(m_penOpacity);
    }
    m_activeStroke.addPoint(point);
    m_hasActiveStroke = true;
    update();
}

void InkCanvas::onStrokeUpdated(const InputPoint &point)
{
    if (m_currentTool == ToolType::Eraser) {
        handleErase(point.toPointF());
        return;
    }

    if (m_currentTool == ToolType::Lasso) {
        m_activeLassoPath.lineTo(point.toPointF());
        update();
        return;
    }

    if (m_currentTool == ToolType::LaserPointer) {
        m_laserPoints.push_back(point);
        if (m_laserPoints.size() > 30) {
            m_laserPoints.erase(m_laserPoints.begin());
        }
        m_laserOpacity = 1.0;
        update();
        return;
    }

    if (m_currentTool == ToolType::Shape) {
        m_activeStroke = Stroke(ToolType::Pen);
        m_activeStroke.setColor(m_penColor);
        m_activeStroke.setBaseWidth(m_penWidth);
        m_activeStroke.setOpacity(m_penOpacity);
        m_activeStroke.addPoint(InputPoint{m_shapeStartPos.x(), m_shapeStartPos.y(), 1.0, 0});
        m_activeStroke.addPoint(point);
        update();
        return;
    }

    m_activeStroke.addPoint(point);
    update();
}

void InkCanvas::onStrokeFinished(const InputPoint &point)
{
    if (m_currentTool == ToolType::Eraser) {
        m_hasActiveStroke = false;
        return;
    }

    if (m_currentTool == ToolType::Lasso) {
        m_activeLassoPath.closeSubpath();
        handleLassoSelect(m_activeLassoPath);
        m_activeLassoPath = QPainterPath();
        m_hasActiveStroke = false;
        update();
        return;
    }

    if (m_currentTool == ToolType::LaserPointer) {
        m_hasActiveStroke = false;
        return;
    }

    if (m_currentTool == ToolType::Shape) {
        finishShapeStroke(m_shapeStartPos, point.toPointF());
        m_hasActiveStroke = false;
        m_activeStroke = Stroke();
        update();
        return;
    }

    if (!m_activeStroke.isEmpty() && currentPage()) {
        m_undoStack.push(new AddStrokeCommand(currentPage(), m_activeStroke));
        m_dirty = true;
        emit pageModified();
    }

    m_hasActiveStroke = false;
    m_activeStroke = Stroke();
    update();
}

void InkCanvas::handleErase(const QPointF &pagePos)
{
    Page *p = currentPage();
    if (!p) return;

    double radius = m_eraserWidth * 0.5;
    QRectF eraseRect(pagePos.x() - radius, pagePos.y() - radius, radius * 2.0, radius * 2.0);

    for (int i = p->strokeCount() - 1; i >= 0; --i) {
        Stroke *s = p->strokeById(p->strokes()[i].id());
        if (s && s->containsPoint(pagePos, radius)) {
            m_undoStack.push(new RemoveStrokeCommand(p, s->id()));
            m_dirty = true;
            emit pageModified();
            update();
            break;
        }
    }
}

void InkCanvas::handleLassoSelect(const QPainterPath &lassoPath)
{
    m_selectedStrokeIds.clear();
    Page *p = currentPage();
    if (!p) {
        emit selectionChanged();
        return;
    }

    for (const auto &s : p->strokes()) {
        if (s.toolType() == ToolType::Pen && !m_selectPenStrokes) continue;
        if (s.toolType() == ToolType::Highlighter && !m_selectHighlighterStrokes) continue;

        // Check if stroke points fall within lasso path
        bool anyContained = false;
        for (const auto &pt : s.points()) {
            if (lassoPath.contains(pt.toPointF())) {
                anyContained = true;
                break;
            }
        }
        if (anyContained) {
            m_selectedStrokeIds.push_back(s.id());
        }
    }

    emit selectionChanged();
    update();
}

void InkCanvas::finishShapeStroke(const QPointF &startPt, const QPointF &endPt)
{
    Page *p = currentPage();
    if (!p) return;

    Stroke shapeStroke(ToolType::Pen);
    shapeStroke.setColor(m_penColor);
    shapeStroke.setBaseWidth(m_penWidth);
    shapeStroke.setOpacity(m_penOpacity);

    QPointF p1 = startPt;
    QPointF p2 = endPt;

    // Optional angle snapping to 0, 45, 90, 135, 180 degrees
    if (m_snapAngles) {
        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double angle = std::atan2(dy, dx) * 180.0 / M_PI;
        double dist = std::hypot(dx, dy);
        double snappedAngle = std::round(angle / 45.0) * 45.0;
        double rad = snappedAngle * M_PI / 180.0;
        p2 = p1 + QPointF(dist * std::cos(rad), dist * std::sin(rad));
    }

    switch (m_shapeType) {
    case ShapeType::Line: {
        shapeStroke.addPoint(p1.x(), p1.y(), 1.0, 0);
        shapeStroke.addPoint(p2.x(), p2.y(), 1.0, 0);
        break;
    }
    case ShapeType::Arrow: {
        shapeStroke.addPoint(p1.x(), p1.y(), 1.0, 0);
        shapeStroke.addPoint(p2.x(), p2.y(), 1.0, 0);

        // Arrow head
        double angle = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
        double headLen = 15.0;
        QPointF a1 = p2 - QPointF(headLen * std::cos(angle - M_PI / 6.0), headLen * std::sin(angle - M_PI / 6.0));
        QPointF a2 = p2 - QPointF(headLen * std::cos(angle + M_PI / 6.0), headLen * std::sin(angle + M_PI / 6.0));
        shapeStroke.addPoint(a1.x(), a1.y(), 1.0, 0);
        shapeStroke.addPoint(p2.x(), p2.y(), 1.0, 0);
        shapeStroke.addPoint(a2.x(), a2.y(), 1.0, 0);
        break;
    }
    case ShapeType::Rectangle: {
        shapeStroke.addPoint(p1.x(), p1.y(), 1.0, 0);
        shapeStroke.addPoint(p2.x(), p1.y(), 1.0, 0);
        shapeStroke.addPoint(p2.x(), p2.y(), 1.0, 0);
        shapeStroke.addPoint(p1.x(), p2.y(), 1.0, 0);
        shapeStroke.addPoint(p1.x(), p1.y(), 1.0, 0);
        break;
    }
    case ShapeType::Circle:
    case ShapeType::Ellipse: {
        QPointF center = (p1 + p2) * 0.5;
        double rx = std::abs(p2.x() - p1.x()) * 0.5;
        double ry = std::abs(p2.y() - p1.y()) * 0.5;
        if (m_shapeType == ShapeType::Circle) {
            rx = ry = std::max(rx, ry);
        }
        int steps = 36;
        for (int i = 0; i <= steps; ++i) {
            double th = i * 2.0 * M_PI / steps;
            shapeStroke.addPoint(center.x() + rx * std::cos(th), center.y() + ry * std::sin(th), 1.0, 0);
        }
        break;
    }
    case ShapeType::Triangle: {
        QPointF top((p1.x() + p2.x()) * 0.5, p1.y());
        shapeStroke.addPoint(top.x(), top.y(), 1.0, 0);
        shapeStroke.addPoint(p2.x(), p2.y(), 1.0, 0);
        shapeStroke.addPoint(p1.x(), p2.y(), 1.0, 0);
        shapeStroke.addPoint(top.x(), top.y(), 1.0, 0);
        break;
    }
    }

    m_undoStack.push(new AddStrokeCommand(p, shapeStroke));
    m_dirty = true;
    emit pageModified();
}

void InkCanvas::paint(QPainter *painter)
{
    Page *p = currentPage();
    if (!p) return;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Fill workspace canvas background (neutral light/dark gray)
    painter->fillRect(boundingRect(), QColor(240, 242, 245));

    // Apply zoom and pan transform
    painter->save();
    painter->translate(m_panOffset);
    painter->scale(m_zoomLevel, m_zoomLevel);

    // 1. Render Paper Background
    m_strokeRenderer.renderPaperBackground(painter, p->paperConfig(), p->width(), p->height(), m_zoomLevel);

    // 2. Render PDF background if present
    if (p->hasPdfBackground() && !p->pdfBackground().isNull()) {
        painter->drawImage(QRectF(0, 0, p->width(), p->height()), p->pdfBackground());
    }

    // 3. Render committed strokes
    for (const auto &s : p->strokes()) {
        m_strokeRenderer.renderStroke(painter, s, m_zoomLevel);
    }

    // 4. Render active stroke in progress
    if (m_hasActiveStroke && !m_activeStroke.isEmpty()) {
        m_strokeRenderer.renderActiveStroke(painter, m_activeStroke, 0, m_zoomLevel);
    }

    // 5. Render active lasso path
    if (m_hasActiveStroke && m_currentTool == ToolType::Lasso && !m_activeLassoPath.isEmpty()) {
        painter->save();
        QPen lassoPen(QColor(33, 150, 243, 200), 1.5, Qt::DashLine);
        painter->setPen(lassoPen);
        painter->setBrush(QBrush(QColor(33, 150, 243, 30)));
        painter->drawPath(m_activeLassoPath);
        painter->restore();
    }

    // 6. Render lasso selection bounding box & handles
    if (hasSelection() && m_currentTool == ToolType::Lasso) {
        QRectF selBox;
        for (const auto &id : m_selectedStrokeIds) {
            Stroke *s = p->strokeById(id);
            if (s) selBox = selBox.united(s->boundingRect());
        }

        if (!selBox.isEmpty()) {
            painter->save();
            selBox.adjust(-4, -4, 4, 4);
            QPen boxPen(QColor(33, 150, 243), 1.5, Qt::DashLine);
            painter->setPen(boxPen);
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(selBox);

            // Corner handles
            painter->setBrush(QBrush(QColor(33, 150, 243)));
            double handleSize = 6.0;
            painter->drawRect(QRectF(selBox.left() - handleSize/2, selBox.top() - handleSize/2, handleSize, handleSize));
            painter->drawRect(QRectF(selBox.right() - handleSize/2, selBox.top() - handleSize/2, handleSize, handleSize));
            painter->drawRect(QRectF(selBox.left() - handleSize/2, selBox.bottom() - handleSize/2, handleSize, handleSize));
            painter->drawRect(QRectF(selBox.right() - handleSize/2, selBox.bottom() - handleSize/2, handleSize, handleSize));
            painter->restore();
        }
    }

    // 7. Render Laser Pointer Trail
    if (!m_laserPoints.empty() && m_laserOpacity > 0.0) {
        m_strokeRenderer.renderLaserTrail(painter, m_laserPoints, m_laserColor, m_laserOpacity, m_laserWidth);
    }

    // 8. Render Ruler overlay if visible
    if (m_rulerVisible) {
        painter->save();
        painter->translate(m_rulerPos);
        painter->rotate(m_rulerAngle);

        QRectF rulerRect(0, -25, 400, 50);
        painter->setPen(QPen(QColor(50, 50, 50, 200), 1.5));
        painter->setBrush(QBrush(QColor(255, 255, 255, 220)));
        painter->drawRoundedRect(rulerRect, 4, 4);

        // Tick marks and measurements
        painter->setPen(QPen(QColor(80, 80, 80), 1.0));
        for (int x = 10; x <= 390; x += 10) {
            double h = (x % 50 == 0) ? 18.0 : (x % 25 == 0 ? 12.0 : 7.0);
            painter->drawLine(QPointF(x, -25), QPointF(x, -25 + h));
            if (x % 50 == 0) {
                painter->drawText(QPointF(x - 5, 5), QString::number(x / 10));
            }
        }
        painter->restore();
    }

    // 9. Render Protractor overlay if visible
    if (m_protractorVisible) {
        painter->save();
        painter->translate(m_protractorPos);

        double radius = 120.0;
        painter->setPen(QPen(QColor(50, 50, 50, 200), 1.5));
        painter->setBrush(QBrush(QColor(255, 255, 255, 220)));
        painter->drawPie(QRectF(-radius, -radius, radius * 2, radius * 2), 0, 180 * 16);

        painter->setPen(QPen(QColor(80, 80, 80), 1.0));
        for (int deg = 0; deg <= 180; deg += 10) {
            double rad = deg * M_PI / 180.0;
            double innerR = (deg % 30 == 0) ? radius - 15.0 : radius - 8.0;
            QPointF pA(radius * std::cos(rad), -radius * std::sin(rad));
            QPointF pB(innerR * std::cos(rad), -innerR * std::sin(rad));
            painter->drawLine(pA, pB);
        }
        painter->restore();
    }

    painter->restore();
}

// Lasso selection actions
void InkCanvas::deleteSelected()
{
    Page *p = currentPage();
    if (!p || m_selectedStrokeIds.empty()) return;

    for (const auto &id : m_selectedStrokeIds) {
        m_undoStack.push(new RemoveStrokeCommand(p, id));
    }
    m_selectedStrokeIds.clear();
    m_dirty = true;
    emit selectionChanged();
    emit pageModified();
    update();
}

void InkCanvas::recolorSelected(const QColor &color)
{
    Page *p = currentPage();
    if (!p || m_selectedStrokeIds.empty()) return;

    for (const auto &id : m_selectedStrokeIds) {
        Stroke *s = p->strokeById(id);
        if (s) {
            s->setColor(color);
        }
    }
    m_dirty = true;
    emit pageModified();
    update();
}

void InkCanvas::copySelected()
{
    m_clipboardStrokes.clear();
    Page *p = currentPage();
    if (!p) return;

    for (const auto &id : m_selectedStrokeIds) {
        Stroke *s = p->strokeById(id);
        if (s) {
            m_clipboardStrokes.push_back(*s);
        }
    }
}

void InkCanvas::pasteSelected()
{
    Page *p = currentPage();
    if (!p || m_clipboardStrokes.empty()) return;

    m_selectedStrokeIds.clear();
    QPointF offset(20.0, 20.0);

    for (const auto &orig : m_clipboardStrokes) {
        Stroke copy = orig;
        copy.translate(offset);
        m_undoStack.push(new AddStrokeCommand(p, copy));
        m_selectedStrokeIds.push_back(copy.id());
    }

    m_dirty = true;
    emit selectionChanged();
    emit pageModified();
    update();
}

void InkCanvas::clearSelection()
{
    if (!m_selectedStrokeIds.empty()) {
        m_selectedStrokeIds.clear();
        emit selectionChanged();
        update();
    }
}

// Notebook and Page CRUD
void InkCanvas::setNotebook(const QString &notebookJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(notebookJson.toUtf8());
    if (doc.isObject()) {
        m_notebook = std::make_unique<Notebook>(Notebook::fromJson(doc.object()));
        m_currentPageIndex = 0;
        m_undoStack.clear();
        clearSelection();
        emit notebookChanged();
        emit pageCountChanged();
        emit currentPageIndexChanged();
        emit pageDimensionsChanged();
        emit paperTypeChanged();
        emit paperColorChanged();
        update();
    }
}

QString InkCanvas::getNotebookJson() const
{
    if (!m_notebook) return QString();
    return QString::fromUtf8(QJsonDocument(m_notebook->toJson()).toJson(QJsonDocument::Compact));
}

void InkCanvas::newNotebook(const QString &title)
{
    m_notebook = std::make_unique<Notebook>(title.isEmpty() ? "Untitled Notebook" : title);
    m_notebook->addPage(Page(PageSize::A4));
    m_currentPageIndex = 0;
    m_undoStack.clear();
    clearSelection();
    emit notebookChanged();
    emit pageCountChanged();
    emit currentPageIndexChanged();
    emit pageDimensionsChanged();
    emit paperTypeChanged();
    emit paperColorChanged();
    update();
}

void InkCanvas::addPage()
{
    if (!m_notebook) return;
    m_notebook->addPage(Page(PageSize::A4));
    m_currentPageIndex = m_notebook->pageCount() - 1;
    clearSelection();
    emit pageCountChanged();
    emit currentPageIndexChanged();
    emit pageDimensionsChanged();
    emit paperTypeChanged();
    emit paperColorChanged();
    m_dirty = true;
    update();
}

void InkCanvas::deletePage(int index)
{
    if (!m_notebook || m_notebook->pageCount() <= 1) return;
    m_notebook->removePage(index);
    if (m_currentPageIndex >= m_notebook->pageCount()) {
        m_currentPageIndex = m_notebook->pageCount() - 1;
    }
    clearSelection();
    emit pageCountChanged();
    emit currentPageIndexChanged();
    emit pageDimensionsChanged();
    emit paperTypeChanged();
    emit paperColorChanged();
    m_dirty = true;
    update();
}

void InkCanvas::duplicateCurrentPage()
{
    Page *p = currentPage();
    if (!p || !m_notebook) return;

    Page dup = *p;
    m_notebook->insertPage(m_currentPageIndex + 1, dup);
    m_currentPageIndex++;
    emit pageCountChanged();
    emit currentPageIndexChanged();
    m_dirty = true;
    update();
}

void InkCanvas::movePage(int from, int to)
{
    if (!m_notebook) return;
    m_notebook->movePage(from, to);
    m_currentPageIndex = to;
    emit currentPageIndexChanged();
    m_dirty = true;
    update();
}

void InkCanvas::nextPage()
{
    if (!m_notebook || m_currentPageIndex >= m_notebook->pageCount() - 1) return;
    setCurrentPageIndex(m_currentPageIndex + 1);
}

void InkCanvas::previousPage()
{
    if (m_currentPageIndex <= 0) return;
    setCurrentPageIndex(m_currentPageIndex - 1);
}

void InkCanvas::undo()
{
    if (m_undoStack.canUndo()) {
        m_undoStack.undo();
        m_dirty = true;
        emit pageModified();
        update();
    }
}

void InkCanvas::redo()
{
    if (m_undoStack.canRedo()) {
        m_undoStack.redo();
        m_dirty = true;
        emit pageModified();
        update();
    }
}

void InkCanvas::clearPage()
{
    Page *p = currentPage();
    if (!p) return;

    for (int i = p->strokeCount() - 1; i >= 0; --i) {
        m_undoStack.push(new RemoveStrokeCommand(p, p->strokes()[i].id()));
    }
    m_dirty = true;
    emit pageModified();
    update();
}

void InkCanvas::zoomIn()
{
    setZoomLevel(m_zoomLevel * 1.25);
}

void InkCanvas::zoomOut()
{
    setZoomLevel(m_zoomLevel * 0.8);
}

void InkCanvas::zoomFit()
{
    Page *p = currentPage();
    if (!p || width() <= 0 || height() <= 0) return;

    double zoomW = (width() - 80) / p->width();
    double zoomH = (height() - 80) / p->height();
    double fitZoom = std::min(zoomW, zoomH);

    m_zoomLevel = std::max(0.1, std::min(3.0, fitZoom));
    m_panOffset = QPointF((width() - p->width() * m_zoomLevel) * 0.5,
                          (height() - p->height() * m_zoomLevel) * 0.5);

    emit zoomLevelChanged();
    emit panOffsetChanged();
    update();
}

void InkCanvas::onAutoSave()
{
    if (m_dirty) {
        emit pageModified();
        m_dirty = false;
    }
}

// PDF Import & Export
bool InkCanvas::importPdf(const QString &filePath)
{
    // Using pdftoppm (standard on Fedora / Linux poppler-utils) to render PDF pages into images
    QString cleanPath = filePath;
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }

    if (!QFileInfo::exists(cleanPath)) {
        return false;
    }

    QString tempDir = QDir::tempPath() + "/inkwell_pdf_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    QDir().mkpath(tempDir);

    QProcess process;
    QStringList args;
    args << "-png" << "-r" << "150" << cleanPath << (tempDir + "/page");
    process.start("pdftoppm", args);
    process.waitForFinished(30000);

    QDir dir(tempDir);
    QStringList filters;
    filters << "page-*.png" << "page.png";
    QStringList renderedPages = dir.entryList(filters, QDir::Files, QDir::Name);

    if (renderedPages.isEmpty()) {
        // Fallback: create empty page
        return false;
    }

    if (!m_notebook) {
        m_notebook = std::make_unique<Notebook>(QFileInfo(cleanPath).baseName());
    }

    for (int i = 0; i < renderedPages.size(); ++i) {
        QImage img(dir.filePath(renderedPages[i]));
        if (!img.isNull()) {
            Page pdfPage;
            pdfPage.setPdfBackground(img);
            m_notebook->addPage(pdfPage);
        }
    }

    // Clean up temporary files
    dir.removeRecursively();

    m_currentPageIndex = 0;
    emit notebookChanged();
    emit pageCountChanged();
    emit currentPageIndexChanged();
    emit pageDimensionsChanged();
    update();
    return true;
}

bool InkCanvas::exportPdf(const QString &filePath, bool includeBackground)
{
    if (!m_notebook || m_notebook->pageCount() == 0) return false;

    QString cleanPath = filePath;
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }

    QPdfWriter writer(cleanPath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);

    QPainter painter(&writer);
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < m_notebook->pageCount(); ++i) {
        if (i > 0) {
            writer.newPage();
        }

        Page *p = m_notebook->page(i);
        if (!p) continue;

        QRectF targetRect(0, 0, writer.width(), writer.height());
        double scaleX = static_cast<double>(writer.width()) / p->width();
        double scaleY = static_cast<double>(writer.height()) / p->height();

        painter.save();
        painter.scale(scaleX, scaleY);

        if (includeBackground) {
            m_strokeRenderer.renderPaperBackground(&painter, p->paperConfig(), p->width(), p->height(), 1.0);
        }

        if (p->hasPdfBackground() && !p->pdfBackground().isNull()) {
            painter.drawImage(QRectF(0, 0, p->width(), p->height()), p->pdfBackground());
        }

        for (const auto &s : p->strokes()) {
            m_strokeRenderer.renderStroke(&painter, s, 1.0);
        }

        painter.restore();
    }

    painter.end();
    return true;
}

bool InkCanvas::exportPageImage(const QString &filePath, int pageIndex)
{
    int idx = (pageIndex < 0) ? m_currentPageIndex : pageIndex;
    if (!m_notebook || idx < 0 || idx >= m_notebook->pageCount()) return false;

    Page *p = m_notebook->page(idx);
    if (!p) return false;

    QImage image(static_cast<int>(p->width() * 2), static_cast<int>(p->height() * 2), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.scale(2.0, 2.0);

    m_strokeRenderer.renderPaperBackground(&painter, p->paperConfig(), p->width(), p->height(), 1.0);
    if (p->hasPdfBackground() && !p->pdfBackground().isNull()) {
        painter.drawImage(QRectF(0, 0, p->width(), p->height()), p->pdfBackground());
    }
    for (const auto &s : p->strokes()) {
        m_strokeRenderer.renderStroke(&painter, s, 1.0);
    }
    painter.end();

    QString cleanPath = filePath;
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }
    return image.save(cleanPath);
}

// Local OCR execution via Tesseract CLI
QString InkCanvas::performOcrOnSelection()
{
    Page *p = currentPage();
    if (!p || m_selectedStrokeIds.empty()) return QString("No strokes selected for OCR.");

    QRectF selBox;
    for (const auto &id : m_selectedStrokeIds) {
        Stroke *s = p->strokeById(id);
        if (s) selBox = selBox.united(s->boundingRect());
    }

    if (selBox.isEmpty()) return QString("Selection is empty.");

    QImage img(static_cast<int>(selBox.width() * 2), static_cast<int>(selBox.height() * 2), QImage::Format_RGB32);
    img.fill(Qt::white);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.scale(2.0, 2.0);
    painter.translate(-selBox.topLeft());

    for (const auto &id : m_selectedStrokeIds) {
        Stroke *s = p->strokeById(id);
        if (s) m_strokeRenderer.renderStroke(&painter, *s, 1.0);
    }
    painter.end();

    QString tempImg = QDir::tempPath() + "/inkwell_ocr_" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".png";
    img.save(tempImg);

    QProcess proc;
    proc.start("tesseract", QStringList() << tempImg << "stdout" << "--oem" << "1");
    proc.waitForFinished(10000);
    QFile::remove(tempImg);

    QString text = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    return text.isEmpty() ? QString("[Handwriting OCR: No text recognized]") : text;
}

QString InkCanvas::performOcrOnPage()
{
    Page *p = currentPage();
    if (!p || p->strokeCount() == 0) return QString("Page is empty.");

    QImage img(static_cast<int>(p->width()), static_cast<int>(p->height()), QImage::Format_RGB32);
    img.fill(Qt::white);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, true);
    for (const auto &s : p->strokes()) {
        m_strokeRenderer.renderStroke(&painter, s, 1.0);
    }
    painter.end();

    QString tempImg = QDir::tempPath() + "/inkwell_page_ocr_" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".png";
    img.save(tempImg);

    QProcess proc;
    proc.start("tesseract", QStringList() << tempImg << "stdout" << "--oem" << "1");
    proc.waitForFinished(10000);
    QFile::remove(tempImg);

    QString text = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    return text.isEmpty() ? QString("[Handwriting OCR: No text recognized]") : text;
}

} // namespace Inkwell
