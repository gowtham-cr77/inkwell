#include "StrokeRenderer.h"
#include <QPen>
#include <QBrush>
#include <cmath>

namespace Inkwell {

StrokeRenderer::StrokeRenderer()
{
}

void StrokeRenderer::renderStroke(QPainter *painter, const Stroke &stroke, double zoomFactor)
{
    if (stroke.isEmpty()) return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    switch (stroke.toolType()) {
    case ToolType::Pen:
        renderPenStroke(painter, stroke, zoomFactor);
        break;
    case ToolType::Highlighter:
        renderHighlighterStroke(painter, stroke, zoomFactor);
        break;
    default:
        renderSimpleStroke(painter, stroke, zoomFactor);
        break;
    }

    painter->restore();
}

void StrokeRenderer::renderPenStroke(QPainter *painter, const Stroke &stroke, double /*zoomFactor*/)
{
    QPainterPath ribbon = stroke.variableWidthPath();
    QColor c = stroke.color();
    c.setAlphaF(stroke.opacity());

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(c));
    painter->drawPath(ribbon);
}

void StrokeRenderer::renderHighlighterStroke(QPainter *painter, const Stroke &stroke, double /*zoomFactor*/)
{
    // Highlighter renders with Multiply composition mode or semi-transparent flat color
    painter->setCompositionMode(QPainter::CompositionMode_Multiply);

    QPainterPath path = stroke.smoothPath();
    QColor c = stroke.color();
    // Default highlighter opacity ~0.35-0.4
    double alpha = stroke.opacity() * 0.4;
    c.setAlphaF(alpha);

    QPen pen(c, stroke.baseWidth(), Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
}

void StrokeRenderer::renderSimpleStroke(QPainter *painter, const Stroke &stroke, double /*zoomFactor*/)
{
    QPainterPath path = stroke.smoothPath();
    QColor c = stroke.color();
    c.setAlphaF(stroke.opacity());

    QPen pen(c, stroke.baseWidth(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
}

void StrokeRenderer::renderActiveStroke(QPainter *painter, const Stroke &stroke, int /*fromPointIndex*/, double zoomFactor)
{
    // For live stroke rendering with minimum latency, render variable width path
    renderStroke(painter, stroke, zoomFactor);
}

void StrokeRenderer::renderPaperBackground(QPainter *painter, const PaperConfig &config,
                                           double pageWidth, double pageHeight, double zoomFactor)
{
    painter->save();
    QRectF pageRect(0, 0, pageWidth, pageHeight);

    // Page drop shadow
    QRectF shadowRect = pageRect.translated(3, 3);
    painter->fillRect(shadowRect, QColor(0, 0, 0, 30));

    // Fill page background
    painter->fillRect(pageRect, config.backgroundColor);

    // Draw page border
    painter->setPen(QPen(QColor(210, 210, 215), 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(pageRect);

    painter->setClipRect(pageRect);

    switch (config.type) {
    case PaperType::Ruled:
        drawRuledLines(painter, config, pageWidth, pageHeight, zoomFactor);
        break;
    case PaperType::Grid:
        drawGrid(painter, config, pageWidth, pageHeight, zoomFactor);
        break;
    case PaperType::Dotted:
        drawDots(painter, config, pageWidth, pageHeight, zoomFactor);
        break;
    case PaperType::Cornell:
        drawCornell(painter, config, pageWidth, pageHeight, zoomFactor);
        break;
    case PaperType::Blank:
    default:
        break;
    }

    painter->restore();
}

void StrokeRenderer::drawRuledLines(QPainter *painter, const PaperConfig &config,
                                    double width, double height, double /*zoom*/)
{
    painter->setPen(QPen(config.lineColor, 0.75));
    double spacing = config.lineSpacing;
    double startY = config.margin;

    for (double y = startY; y < height - 10.0; y += spacing) {
        painter->drawLine(QPointF(0, y), QPointF(width, y));
    }

    // Left vertical margin line (pink/red rule)
    if (config.margin > 10.0) {
        painter->setPen(QPen(QColor(235, 140, 140), 1.0));
        painter->drawLine(QPointF(config.margin * 1.5, 0), QPointF(config.margin * 1.5, height));
    }
}

void StrokeRenderer::drawGrid(QPainter *painter, const PaperConfig &config,
                              double width, double height, double /*zoom*/)
{
    painter->setPen(QPen(config.lineColor, 0.5));
    double size = config.gridSize;

    for (double x = size; x < width; x += size) {
        painter->drawLine(QPointF(x, 0), QPointF(x, height));
    }
    for (double y = size; y < height; y += size) {
        painter->drawLine(QPointF(0, y), QPointF(width, y));
    }
}

void StrokeRenderer::drawDots(QPainter *painter, const PaperConfig &config,
                              double width, double height, double /*zoom*/)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(config.lineColor));
    double size = config.gridSize;
    double dotRadius = 1.0;

    for (double x = size; x < width; x += size) {
        for (double y = size; y < height; y += size) {
            painter->drawEllipse(QPointF(x, y), dotRadius, dotRadius);
        }
    }
}

void StrokeRenderer::drawCornell(QPainter *painter, const PaperConfig &config,
                                 double width, double height, double zoom)
{
    // Cornell layout:
    // Cue column: left 25-30%
    // Summary row: bottom 15-20%
    double cueWidth = width * 0.28;
    double summaryHeight = height * 0.18;
    double notesHeight = height - summaryHeight;

    // Draw horizontal ruled lines in notes area
    painter->setPen(QPen(config.lineColor, 0.75));
    double spacing = config.lineSpacing;
    for (double y = 50.0; y < notesHeight; y += spacing) {
        painter->drawLine(QPointF(cueWidth, y), QPointF(width, y));
    }

    // Vertical cue separator
    painter->setPen(QPen(QColor(160, 160, 165), 1.5));
    painter->drawLine(QPointF(cueWidth, 0), QPointF(cueWidth, notesHeight));

    // Horizontal summary separator
    painter->drawLine(QPointF(0, notesHeight), QPointF(width, notesHeight));

    // Label indicators (subtle)
    painter->setPen(QPen(QColor(170, 170, 175), 1.0));
    QFont font = painter->font();
    font.setPointSize(9);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(QRectF(10, 10, cueWidth - 20, 25), Qt::AlignLeft | Qt::AlignVCenter, "CUE / TOPICS");
    painter->drawText(QRectF(cueWidth + 10, 10, width - cueWidth - 20, 25), Qt::AlignLeft | Qt::AlignVCenter, "NOTES");
    painter->drawText(QRectF(10, notesHeight + 5, width - 20, 25), Qt::AlignLeft | Qt::AlignVCenter, "SUMMARY");
}

void StrokeRenderer::renderLaserTrail(QPainter *painter, const std::vector<InputPoint> &points,
                                      const QColor &color, double opacity, double width)
{
    if (points.empty()) return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // Glowing dot at the current tip
    QPointF tip = points.back().toPointF();
    QColor glowColor = color;
    glowColor.setAlphaF(opacity * 0.4);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(glowColor));
    painter->drawEllipse(tip, width * 1.5, width * 1.5);

    QColor coreColor = Qt::white;
    coreColor.setAlphaF(opacity);
    painter->setBrush(QBrush(coreColor));
    painter->drawEllipse(tip, width * 0.5, width * 0.5);

    // Trail lines with fading opacity
    if (points.size() > 1) {
        size_t n = points.size();
        for (size_t i = 1; i < n; ++i) {
            double progress = static_cast<double>(i) / n;
            QColor segColor = color;
            segColor.setAlphaF(opacity * progress * 0.7);

            double segWidth = width * (0.3 + 0.7 * progress);
            QPen pen(segColor, segWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(pen);
            painter->drawLine(points[i - 1].toPointF(), points[i].toPointF());
        }
    }

    painter->restore();
}

} // namespace Inkwell
