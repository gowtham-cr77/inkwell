#pragma once

#include "Types.h"
#include <QObject>
#include <QPainterPath>
#include <QRectF>

namespace Inkwell {

class Stroke {
public:
    Stroke();
    explicit Stroke(ToolType tool);

    // Identification
    QString id() const { return m_id; }

    // Tool properties
    ToolType toolType() const { return m_toolType; }
    void setToolType(ToolType type) { m_toolType = type; }

    QColor color() const { return m_color; }
    void setColor(const QColor &color) { m_color = color; }

    double baseWidth() const { return m_baseWidth; }
    void setBaseWidth(double width) { m_baseWidth = width; }

    double opacity() const { return m_opacity; }
    void setOpacity(double opacity) { m_opacity = opacity; }

    // Points
    void addPoint(const InputPoint &point);
    void addPoint(double x, double y, double pressure, qint64 timestamp);
    const std::vector<InputPoint>& points() const { return m_points; }
    int pointCount() const { return static_cast<int>(m_points.size()); }
    bool isEmpty() const { return m_points.empty(); }

    // Geometry
    QRectF boundingRect() const;
    bool containsPoint(const QPointF &point, double tolerance = 5.0) const;
    bool intersectsRect(const QRectF &rect) const;

    // Rendering - generates a smoothed QPainterPath
    QPainterPath smoothPath() const;

    // Generates variable-width outline path for pressure-sensitive rendering
    QPainterPath variableWidthPath() const;

    // Transform
    void translate(const QPointF &delta);
    void scale(double sx, double sy, const QPointF &center);
    void rotate(double angle, const QPointF &center);

    // Serialization
    QJsonObject toJson() const;
    static Stroke fromJson(const QJsonObject &json);

private:
    // Smoothing helpers
    QPointF catmullRomPoint(const QPointF &p0, const QPointF &p1,
                            const QPointF &p2, const QPointF &p3,
                            double t, double alpha = 0.5) const;
    std::vector<QPointF> generateSmoothedPoints(int samplesPerSegment = 8) const;
    double widthAtPressure(double pressure) const;

    QString m_id;
    ToolType m_toolType = ToolType::Pen;
    QColor m_color = QColor(0, 0, 0);
    double m_baseWidth = 2.5;
    double m_opacity = 1.0;
    std::vector<InputPoint> m_points;

    // Cached bounding rect
    mutable QRectF m_boundingRect;
    mutable bool m_boundingRectDirty = true;
};

} // namespace Inkwell
