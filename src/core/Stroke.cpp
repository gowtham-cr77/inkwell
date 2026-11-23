#include "Stroke.h"
#include <QPainterPathStroker>
#include <QTransform>
#include <cmath>

namespace Inkwell {

Stroke::Stroke()
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_toolType(ToolType::Pen)
{
}

Stroke::Stroke(ToolType tool)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_toolType(tool)
{
}

void Stroke::addPoint(const InputPoint &point)
{
    m_points.push_back(point);
    m_boundingRectDirty = true;
}

void Stroke::addPoint(double x, double y, double pressure, qint64 timestamp)
{
    InputPoint pt;
    pt.x = x;
    pt.y = y;
    pt.pressure = pressure;
    pt.timestamp = timestamp;
    addPoint(pt);
}

QRectF Stroke::boundingRect() const
{
    if (!m_boundingRectDirty) {
        return m_boundingRect;
    }

    if (m_points.empty()) {
        m_boundingRect = QRectF();
        m_boundingRectDirty = false;
        return m_boundingRect;
    }

    double minX = m_points[0].x;
    double maxX = m_points[0].x;
    double minY = m_points[0].y;
    double maxY = m_points[0].y;
    double maxW = m_baseWidth * 2.0;

    for (const auto &p : m_points) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    m_boundingRect = QRectF(minX - maxW, minY - maxW, 
                            (maxX - minX) + maxW * 2.0, 
                            (maxY - minY) + maxW * 2.0);
    m_boundingRectDirty = false;
    return m_boundingRect;
}

bool Stroke::containsPoint(const QPointF &point, double tolerance) const
{
    if (m_points.empty()) return false;
    if (!boundingRect().adjusted(-tolerance, -tolerance, tolerance, tolerance).contains(point)) {
        return false;
    }

    double maxDistSq = (tolerance + m_baseWidth) * (tolerance + m_baseWidth);

    for (size_t i = 0; i < m_points.size(); ++i) {
        double dx = m_points[i].x - point.x();
        double dy = m_points[i].y - point.y();
        if ((dx * dx + dy * dy) <= maxDistSq) {
            return true;
        }

        if (i + 1 < m_points.size()) {
            QPointF p1 = m_points[i].toPointF();
            QPointF p2 = m_points[i + 1].toPointF();
            QPointF diff = p2 - p1;
            double lenSq = diff.x() * diff.x() + diff.y() * diff.y();
            if (lenSq > 0.0001) {
                double t = std::max(0.0, std::min(1.0, QPointF::dotProduct(point - p1, diff) / lenSq));
                QPointF proj = p1 + t * diff;
                double dpx = proj.x() - point.x();
                double dpy = proj.y() - point.y();
                if ((dpx * dpx + dpy * dpy) <= maxDistSq) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool Stroke::intersectsRect(const QRectF &rect) const
{
    if (m_points.empty()) return false;
    if (!boundingRect().intersects(rect)) return false;

    for (const auto &p : m_points) {
        if (rect.contains(p.toPointF())) return true;
    }

    // Check line segments against rect edges
    for (size_t i = 0; i + 1 < m_points.size(); ++i) {
        QPointF p1 = m_points[i].toPointF();
        QPointF p2 = m_points[i + 1].toPointF();
        
        if (rect.contains(p1) || rect.contains(p2)) return true;
        
        QLineF seg(p1, p2);
        QPointF intersectPt;
        if (seg.intersects(QLineF(rect.topLeft(), rect.topRight()), &intersectPt) == QLineF::BoundedIntersection ||
            seg.intersects(QLineF(rect.topRight(), rect.bottomRight()), &intersectPt) == QLineF::BoundedIntersection ||
            seg.intersects(QLineF(rect.bottomRight(), rect.bottomLeft()), &intersectPt) == QLineF::BoundedIntersection ||
            seg.intersects(QLineF(rect.bottomLeft(), rect.topLeft()), &intersectPt) == QLineF::BoundedIntersection) {
            return true;
        }
    }

    return false;
}

double Stroke::widthAtPressure(double pressure) const
{
    // Gamma power curve (gamma = 1.5) mapping for natural handwriting response
    double minW = m_baseWidth * 0.4;
    double maxW = m_baseWidth * 1.8;
    double p = std::max(0.0, std::min(1.0, pressure));
    return minW + (maxW - minW) * std::pow(p, 1.5);
}

QPointF Stroke::catmullRomPoint(const QPointF &p0, const QPointF &p1,
                                const QPointF &p2, const QPointF &p3,
                                double t, double alpha) const
{
    auto getT = [](double tPrev, const QPointF &pA, const QPointF &pB, double a) -> double {
        double d = std::hypot(pB.x() - pA.x(), pB.y() - pA.y());
        return tPrev + (d > 0.0001 ? std::pow(d, a) : 0.0001);
    };

    double t0 = 0.0;
    double t1 = getT(t0, p0, p1, alpha);
    double t2 = getT(t1, p1, p2, alpha);
    double t3 = getT(t2, p2, p3, alpha);

    double actualT = t1 + t * (t2 - t1);

    auto evalLinear = [](const QPointF &pA, const QPointF &pB, double tA, double tB, double curT) -> QPointF {
        if (std::abs(tB - tA) < 0.00001) return pA;
        return (tB - curT) / (tB - tA) * pA + (curT - tA) / (tB - tA) * pB;
    };

    QPointF a1 = evalLinear(p0, p1, t0, t1, actualT);
    QPointF a2 = evalLinear(p1, p2, t1, t2, actualT);
    QPointF a3 = evalLinear(p2, p3, t2, t3, actualT);

    QPointF b1 = evalLinear(a1, a2, t0, t2, actualT);
    QPointF b2 = evalLinear(a2, a3, t1, t3, actualT);

    return evalLinear(b1, b2, t1, t2, actualT);
}

std::vector<QPointF> Stroke::generateSmoothedPoints(int samplesPerSegment) const
{
    std::vector<QPointF> smoothed;
    if (m_points.empty()) return smoothed;
    if (m_points.size() == 1) {
        smoothed.push_back(m_points[0].toPointF());
        return smoothed;
    }
    if (m_points.size() == 2) {
        smoothed.push_back(m_points[0].toPointF());
        smoothed.push_back(m_points[1].toPointF());
        return smoothed;
    }

    size_t n = m_points.size();
    for (size_t i = 0; i < n - 1; ++i) {
        QPointF p0 = (i == 0) ? m_points[0].toPointF() - (m_points[1].toPointF() - m_points[0].toPointF()) 
                              : m_points[i - 1].toPointF();
        QPointF p1 = m_points[i].toPointF();
        QPointF p2 = m_points[i + 1].toPointF();
        QPointF p3 = (i + 2 < n) ? m_points[i + 2].toPointF() 
                                 : m_points[n - 1].toPointF() + (m_points[n - 1].toPointF() - m_points[n - 2].toPointF());

        for (int s = 0; s < samplesPerSegment; ++s) {
            double t = static_cast<double>(s) / samplesPerSegment;
            smoothed.push_back(catmullRomPoint(p0, p1, p2, p3, t, 0.5));
        }
    }
    smoothed.push_back(m_points.back().toPointF());
    return smoothed;
}

QPainterPath Stroke::smoothPath() const
{
    QPainterPath path;
    if (m_points.empty()) return path;

    if (m_points.size() == 1) {
        double r = m_baseWidth * 0.5;
        path.addEllipse(m_points[0].toPointF(), r, r);
        return path;
    }

    auto pts = generateSmoothedPoints(6);
    if (pts.empty()) return path;

    path.moveTo(pts[0]);
    for (size_t i = 1; i < pts.size(); ++i) {
        path.lineTo(pts[i]);
    }
    return path;
}

QPainterPath Stroke::variableWidthPath() const
{
    QPainterPath ribbon;
    if (m_points.empty()) return ribbon;

    if (m_points.size() == 1) {
        double r = widthAtPressure(m_points[0].pressure) * 0.5;
        ribbon.addEllipse(m_points[0].toPointF(), r, r);
        return ribbon;
    }

    // Centripetal Catmull-Rom smoothed interpolation with variable ribbon width
    int samples = 6;
    size_t n = m_points.size();
    std::vector<QPointF> centerPoints;
    std::vector<double> widths;

    for (size_t i = 0; i < n - 1; ++i) {
        QPointF p0 = (i == 0) ? m_points[0].toPointF() - (m_points[1].toPointF() - m_points[0].toPointF()) 
                              : m_points[i - 1].toPointF();
        QPointF p1 = m_points[i].toPointF();
        QPointF p2 = m_points[i + 1].toPointF();
        QPointF p3 = (i + 2 < n) ? m_points[i + 2].toPointF() 
                                 : m_points[n - 1].toPointF() + (m_points[n - 1].toPointF() - m_points[n - 2].toPointF());

        double w1 = widthAtPressure(m_points[i].pressure);
        double w2 = widthAtPressure(m_points[i + 1].pressure);

        for (int s = 0; s < samples; ++s) {
            double t = static_cast<double>(s) / samples;
            centerPoints.push_back(catmullRomPoint(p0, p1, p2, p3, t, 0.5));
            widths.push_back(w1 + t * (w2 - w1));
        }
    }
    centerPoints.push_back(m_points.back().toPointF());
    widths.push_back(widthAtPressure(m_points.back().pressure));

    if (centerPoints.size() < 2) {
        return smoothPath();
    }

    std::vector<QPointF> leftBoundary;
    std::vector<QPointF> rightBoundary;
    leftBoundary.reserve(centerPoints.size());
    rightBoundary.reserve(centerPoints.size());

    for (size_t i = 0; i < centerPoints.size(); ++i) {
        QPointF tangent;
        if (i == 0) {
            tangent = centerPoints[1] - centerPoints[0];
        } else if (i == centerPoints.size() - 1) {
            tangent = centerPoints[i] - centerPoints[i - 1];
        } else {
            tangent = centerPoints[i + 1] - centerPoints[i - 1];
        }

        double len = std::hypot(tangent.x(), tangent.y());
        QPointF normal(0.0, 0.0);
        if (len > 0.0001) {
            normal = QPointF(-tangent.y() / len, tangent.x() / len);
        }

        double halfW = widths[i] * 0.5;
        leftBoundary.push_back(centerPoints[i] + normal * halfW);
        rightBoundary.push_back(centerPoints[i] - normal * halfW);
    }

    // Build closed ribbon path
    ribbon.moveTo(leftBoundary[0]);
    for (size_t i = 1; i < leftBoundary.size(); ++i) {
        ribbon.lineTo(leftBoundary[i]);
    }

    // Round end cap
    ribbon.lineTo(rightBoundary.back());

    for (int i = static_cast<int>(rightBoundary.size()) - 1; i >= 0; --i) {
        ribbon.lineTo(rightBoundary[i]);
    }

    ribbon.closeSubpath();
    return ribbon;
}

void Stroke::translate(const QPointF &delta)
{
    for (auto &pt : m_points) {
        pt.x += delta.x();
        pt.y += delta.y();
    }
    m_boundingRectDirty = true;
}

void Stroke::scale(double sx, double sy, const QPointF &center)
{
    for (auto &pt : m_points) {
        pt.x = center.x() + (pt.x - center.x()) * sx;
        pt.y = center.y() + (pt.y - center.y()) * sy;
    }
    m_baseWidth *= (sx + sy) * 0.5;
    m_boundingRectDirty = true;
}

void Stroke::rotate(double angleDegrees, const QPointF &center)
{
    double rad = angleDegrees * M_PI / 180.0;
    double cosA = std::cos(rad);
    double sinA = std::sin(rad);

    for (auto &pt : m_points) {
        double dx = pt.x - center.x();
        double dy = pt.y - center.y();
        pt.x = center.x() + dx * cosA - dy * sinA;
        pt.y = center.y() + dx * sinA + dy * cosA;
    }
    m_boundingRectDirty = true;
}

QJsonObject Stroke::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["tool"] = static_cast<int>(m_toolType);
    obj["color"] = m_color.name(QColor::HexArgb);
    obj["baseWidth"] = m_baseWidth;
    obj["opacity"] = m_opacity;

    QJsonArray ptsArray;
    for (const auto &p : m_points) {
        ptsArray.append(p.toJson());
    }
    obj["points"] = ptsArray;

    return obj;
}

Stroke Stroke::fromJson(const QJsonObject &json)
{
    Stroke stroke;
    stroke.m_id = json["id"].toString(QUuid::createUuid().toString(QUuid::WithoutBraces));
    stroke.m_toolType = static_cast<ToolType>(json["tool"].toInt(static_cast<int>(ToolType::Pen)));
    stroke.m_color = QColor(json["color"].toString("#000000"));
    stroke.m_baseWidth = json["baseWidth"].toDouble(2.5);
    stroke.m_opacity = json["opacity"].toDouble(1.0);

    QJsonArray ptsArray = json["points"].toArray();
    stroke.m_points.reserve(ptsArray.size());
    for (const auto &val : ptsArray) {
        stroke.m_points.push_back(InputPoint::fromJson(val.toObject()));
    }
    stroke.m_boundingRectDirty = true;
    return stroke;
}

} // namespace Inkwell
