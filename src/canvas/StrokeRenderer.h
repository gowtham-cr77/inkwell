#pragma once

#include "../core/Stroke.h"
#include "../core/Types.h"
#include <QPainter>
#include <QColor>

namespace Inkwell {

class StrokeRenderer {
public:
    StrokeRenderer();
    
    // Render a complete stroke
    void renderStroke(QPainter *painter, const Stroke &stroke, double zoomFactor = 1.0);
    
    // Render an in-progress stroke (may use optimizations for real-time)
    void renderActiveStroke(QPainter *painter, const Stroke &stroke, 
                           int fromPointIndex = 0, double zoomFactor = 1.0);
    
    // Render paper background
    void renderPaperBackground(QPainter *painter, const PaperConfig &config,
                               double pageWidth, double pageHeight, double zoomFactor = 1.0);
    
    // Render a laser pointer trail
    void renderLaserTrail(QPainter *painter, const std::vector<InputPoint> &points,
                          const QColor &color, double opacity, double width);
    
private:
    void renderPenStroke(QPainter *painter, const Stroke &stroke, double zoomFactor);
    void renderHighlighterStroke(QPainter *painter, const Stroke &stroke, double zoomFactor);
    void renderSimpleStroke(QPainter *painter, const Stroke &stroke, double zoomFactor);
    
    // Paper patterns
    void drawRuledLines(QPainter *painter, const PaperConfig &config, 
                        double width, double height, double zoom);
    void drawGrid(QPainter *painter, const PaperConfig &config,
                  double width, double height, double zoom);
    void drawDots(QPainter *painter, const PaperConfig &config,
                  double width, double height, double zoom);
    void drawCornell(QPainter *painter, const PaperConfig &config,
                     double width, double height, double zoom);
};

} // namespace Inkwell
