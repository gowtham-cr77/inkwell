#pragma once

#include "../core/Types.h"
#include <QObject>
#include <QPointF>
#include <QElapsedTimer>

namespace Inkwell {

// 1€ Filter for input smoothing
class OneEuroFilter {
public:
    OneEuroFilter(double minCutoff = 1.0, double beta = 0.007, double dCutoff = 1.0);
    double filter(double value, double timestamp);
    void reset();
    
private:
    double alpha(double cutoff, double dt) const;
    
    double m_minCutoff;
    double m_beta;
    double m_dCutoff;
    double m_x;
    double m_dx;
    double m_lastTime;
    bool m_initialized;
};

class InputHandler : public QObject {
    Q_OBJECT

public:
    explicit InputHandler(QObject *parent = nullptr);
    
    // Process raw input events
    void handlePenDown(const QPointF &pos, double pressure, qint64 timestamp);
    void handlePenMove(const QPointF &pos, double pressure, qint64 timestamp);
    void handlePenUp(const QPointF &pos, double pressure, qint64 timestamp);
    
    void handleMouseDown(const QPointF &pos, qint64 timestamp);
    void handleMouseMove(const QPointF &pos, qint64 timestamp);
    void handleMouseUp(const QPointF &pos, qint64 timestamp);
    
    // State
    bool isDrawing() const { return m_isDrawing; }
    
    // Smoothing settings
    void setSmoothingEnabled(bool enabled) { m_smoothingEnabled = enabled; }
    bool smoothingEnabled() const { return m_smoothingEnabled; }
    
signals:
    void strokeStarted(const InputPoint &point);
    void strokeUpdated(const InputPoint &point);
    void strokeFinished(const InputPoint &point);
    
private:
    InputPoint filterPoint(double x, double y, double pressure, qint64 timestamp);
    
    bool m_isDrawing = false;
    bool m_smoothingEnabled = true;
    OneEuroFilter m_filterX;
    OneEuroFilter m_filterY;
    OneEuroFilter m_filterPressure;
    QElapsedTimer m_timer;
};

} // namespace Inkwell
