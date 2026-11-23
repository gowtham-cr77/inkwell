#include "InputHandler.h"
#include <cmath>

namespace Inkwell {

// 1€ Filter Implementation
OneEuroFilter::OneEuroFilter(double minCutoff, double beta, double dCutoff)
    : m_minCutoff(minCutoff)
    , m_beta(beta)
    , m_dCutoff(dCutoff)
    , m_x(0.0)
    , m_dx(0.0)
    , m_lastTime(0.0)
    , m_initialized(false)
{
}

void OneEuroFilter::reset()
{
    m_initialized = false;
    m_x = 0.0;
    m_dx = 0.0;
    m_lastTime = 0.0;
}

double OneEuroFilter::alpha(double cutoff, double dt) const
{
    double tau = 1.0 / (2.0 * M_PI * cutoff);
    return 1.0 / (1.0 + tau / dt);
}

double OneEuroFilter::filter(double value, double timestamp)
{
    if (!m_initialized) {
        m_x = value;
        m_dx = 0.0;
        m_lastTime = timestamp;
        m_initialized = true;
        return value;
    }

    double dt = (timestamp - m_lastTime) / 1000.0; // convert ms to seconds
    if (dt <= 0.0001) {
        dt = 0.001;
    }
    m_lastTime = timestamp;

    // Filter derivative
    double rawDx = (value - m_x) / dt;
    double aD = alpha(m_dCutoff, dt);
    m_dx = m_dx + aD * (rawDx - m_dx);

    // Adaptive cutoff based on speed
    double cutoff = m_minCutoff + m_beta * std::abs(m_dx);
    double a = alpha(cutoff, dt);
    m_x = m_x + a * (value - m_x);

    return m_x;
}

// InputHandler Implementation
InputHandler::InputHandler(QObject *parent)
    : QObject(parent)
    , m_filterX(1.2, 0.005, 1.0)
    , m_filterY(1.2, 0.005, 1.0)
    , m_filterPressure(1.0, 0.001, 1.0)
{
    m_timer.start();
}

InputPoint InputHandler::filterPoint(double x, double y, double pressure, qint64 timestamp)
{
    InputPoint pt;
    if (timestamp <= 0) {
        timestamp = m_timer.elapsed();
    }

    if (m_smoothingEnabled) {
        pt.x = m_filterX.filter(x, static_cast<double>(timestamp));
        pt.y = m_filterY.filter(y, static_cast<double>(timestamp));
        pt.pressure = std::max(0.0, std::min(1.0, m_filterPressure.filter(pressure, static_cast<double>(timestamp))));
    } else {
        pt.x = x;
        pt.y = y;
        pt.pressure = std::max(0.0, std::min(1.0, pressure));
    }
    pt.timestamp = timestamp;
    return pt;
}

void InputHandler::handlePenDown(const QPointF &pos, double pressure, qint64 timestamp)
{
    m_isDrawing = true;
    m_filterX.reset();
    m_filterY.reset();
    m_filterPressure.reset();

    InputPoint pt = filterPoint(pos.x(), pos.y(), pressure, timestamp);
    emit strokeStarted(pt);
}

void InputHandler::handlePenMove(const QPointF &pos, double pressure, qint64 timestamp)
{
    if (!m_isDrawing) return;
    InputPoint pt = filterPoint(pos.x(), pos.y(), pressure, timestamp);
    emit strokeUpdated(pt);
}

void InputHandler::handlePenUp(const QPointF &pos, double pressure, qint64 timestamp)
{
    if (!m_isDrawing) return;
    m_isDrawing = false;
    InputPoint pt = filterPoint(pos.x(), pos.y(), pressure, timestamp);
    emit strokeFinished(pt);
}

void InputHandler::handleMouseDown(const QPointF &pos, qint64 timestamp)
{
    handlePenDown(pos, 1.0, timestamp);
}

void InputHandler::handleMouseMove(const QPointF &pos, qint64 timestamp)
{
    handlePenMove(pos, 1.0, timestamp);
}

void InputHandler::handleMouseUp(const QPointF &pos, qint64 timestamp)
{
    handlePenUp(pos, 1.0, timestamp);
}

} // namespace Inkwell
