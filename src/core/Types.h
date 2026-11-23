#pragma once

#include <QPointF>
#include <QColor>
#include <QString>
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>
#include <cmath>

namespace Inkwell {

// Tool types
enum class ToolType {
    Pen,
    Highlighter,
    Eraser,
    LaserPointer,
    Lasso,
    Shape,
    Text,
    None
};

// Shape types
enum class ShapeType {
    Line,
    Arrow,
    Rectangle,
    Circle,
    Triangle,
    Ellipse
};

// Paper background types
enum class PaperType {
    Blank,
    Ruled,
    Grid,
    Dotted,
    Cornell
};

// Page sizes
enum class PageSize {
    A4,
    Letter,
    A5,
    Custom
};

// Eraser mode
enum class EraserMode {
    StrokeEraser,
    PartialEraser
};

// A single input point with metadata
struct InputPoint {
    double x = 0.0;
    double y = 0.0;
    double pressure = 1.0;
    qint64 timestamp = 0;

    QPointF toPointF() const { return QPointF(x, y); }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["x"] = x;
        obj["y"] = y;
        obj["p"] = pressure;
        obj["t"] = timestamp;
        return obj;
    }

    static InputPoint fromJson(const QJsonObject &obj) {
        InputPoint pt;
        pt.x = obj["x"].toDouble();
        pt.y = obj["y"].toDouble();
        pt.pressure = obj["p"].toDouble(1.0);
        pt.timestamp = static_cast<qint64>(obj["t"].toDouble());
        return pt;
    }
};

// Page size dimensions in points (1 point = 1/72 inch)
struct PageDimensions {
    double width;
    double height;

    static PageDimensions forSize(PageSize size) {
        switch (size) {
            case PageSize::A4: return {595.0, 842.0};
            case PageSize::Letter: return {612.0, 792.0};
            case PageSize::A5: return {420.0, 595.0};
            default: return {595.0, 842.0};
        }
    }
};

// Paper background configuration
struct PaperConfig {
    PaperType type = PaperType::Blank;
    QColor backgroundColor = QColor(255, 255, 255);
    QColor lineColor = QColor(200, 200, 200);
    double lineSpacing = 30.0;      // pixels
    double gridSize = 30.0;         // pixels
    double margin = 40.0;           // pixels
};

} // namespace Inkwell
