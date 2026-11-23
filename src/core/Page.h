#pragma once

#include "Types.h"
#include "Stroke.h"
#include <QObject>
#include <QJsonObject>
#include <QImage>
#include <vector>
#include <memory>

namespace Inkwell {

class Page {
public:
    Page();
    explicit Page(PageSize size);

    // Identity
    QString id() const { return m_id; }

    // Dimensions
    PageDimensions dimensions() const { return m_dimensions; }
    void setDimensions(const PageDimensions &dims) { m_dimensions = dims; }
    double width() const { return m_dimensions.width; }
    double height() const { return m_dimensions.height; }

    // Paper configuration
    PaperConfig paperConfig() const { return m_paperConfig; }
    void setPaperConfig(const PaperConfig &config) { m_paperConfig = config; }

    // Strokes
    void addStroke(const Stroke &stroke);
    void removeStroke(const QString &strokeId);
    void removeStrokesInRect(const QRectF &rect);
    Stroke* strokeById(const QString &id);
    const std::vector<Stroke>& strokes() const { return m_strokes; }
    std::vector<Stroke>& strokes() { return m_strokes; }
    int strokeCount() const { return static_cast<int>(m_strokes.size()); }

    // PDF background
    bool hasPdfBackground() const { return m_hasPdfBackground; }
    void setPdfBackground(const QImage &image);
    QImage pdfBackground() const { return m_pdfBackground; }
    void clearPdfBackground();

    // Serialization
    QJsonObject toJson() const;
    static Page fromJson(const QJsonObject &json);

private:
    QString m_id;
    PageDimensions m_dimensions;
    PaperConfig m_paperConfig;
    std::vector<Stroke> m_strokes;
    bool m_hasPdfBackground = false;
    QImage m_pdfBackground;
};

} // namespace Inkwell
