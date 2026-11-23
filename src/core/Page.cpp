#include "Page.h"
#include <QBuffer>

namespace Inkwell {

Page::Page()
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_dimensions(PageDimensions::forSize(PageSize::A4))
{
    m_paperConfig.type = PaperType::Blank;
    m_paperConfig.backgroundColor = QColor(255, 255, 255);
}

Page::Page(PageSize size)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_dimensions(PageDimensions::forSize(size))
{
    m_paperConfig.type = PaperType::Blank;
    m_paperConfig.backgroundColor = QColor(255, 255, 255);
}

void Page::addStroke(const Stroke &stroke)
{
    m_strokes.push_back(stroke);
}

void Page::removeStroke(const QString &strokeId)
{
    for (auto it = m_strokes.begin(); it != m_strokes.end(); ++it) {
        if (it->id() == strokeId) {
            m_strokes.erase(it);
            return;
        }
    }
}

void Page::removeStrokesInRect(const QRectF &rect)
{
    std::vector<Stroke> remaining;
    remaining.reserve(m_strokes.size());
    for (const auto &s : m_strokes) {
        if (!s.intersectsRect(rect)) {
            remaining.push_back(s);
        }
    }
    m_strokes = std::move(remaining);
}

Stroke* Page::strokeById(const QString &id)
{
    for (auto &s : m_strokes) {
        if (s.id() == id) {
            return &s;
        }
    }
    return nullptr;
}

void Page::setPdfBackground(const QImage &image)
{
    m_pdfBackground = image;
    m_hasPdfBackground = !image.isNull();
    if (m_hasPdfBackground) {
        m_dimensions.width = image.width();
        m_dimensions.height = image.height();
    }
}

void Page::clearPdfBackground()
{
    m_pdfBackground = QImage();
    m_hasPdfBackground = false;
}

QJsonObject Page::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["width"] = m_dimensions.width;
    obj["height"] = m_dimensions.height;

    // Paper configuration
    QJsonObject paper;
    paper["type"] = static_cast<int>(m_paperConfig.type);
    paper["bgColor"] = m_paperConfig.backgroundColor.name(QColor::HexArgb);
    paper["lineColor"] = m_paperConfig.lineColor.name(QColor::HexArgb);
    paper["lineSpacing"] = m_paperConfig.lineSpacing;
    paper["gridSize"] = m_paperConfig.gridSize;
    paper["margin"] = m_paperConfig.margin;
    obj["paper"] = paper;

    // Strokes
    QJsonArray strokesArray;
    for (const auto &s : m_strokes) {
        strokesArray.append(s.toJson());
    }
    obj["strokes"] = strokesArray;

    obj["hasPdf"] = m_hasPdfBackground;
    if (m_hasPdfBackground && !m_pdfBackground.isNull()) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        m_pdfBackground.save(&buffer, "PNG");
        obj["pdfPngBase64"] = QString::fromLatin1(bytes.toBase64());
    }

    return obj;
}

Page Page::fromJson(const QJsonObject &json)
{
    Page page;
    page.m_id = json["id"].toString(QUuid::createUuid().toString(QUuid::WithoutBraces));
    page.m_dimensions.width = json["width"].toDouble(595.0);
    page.m_dimensions.height = json["height"].toDouble(842.0);

    if (json.contains("paper")) {
        QJsonObject paper = json["paper"].toObject();
        page.m_paperConfig.type = static_cast<PaperType>(paper["type"].toInt(0));
        page.m_paperConfig.backgroundColor = QColor(paper["bgColor"].toString("#FFFFFF"));
        page.m_paperConfig.lineColor = QColor(paper["lineColor"].toString("#D0D0D0"));
        page.m_paperConfig.lineSpacing = paper["lineSpacing"].toDouble(30.0);
        page.m_paperConfig.gridSize = paper["gridSize"].toDouble(30.0);
        page.m_paperConfig.margin = paper["margin"].toDouble(40.0);
    }

    if (json.contains("strokes")) {
        QJsonArray strokesArray = json["strokes"].toArray();
        page.m_strokes.reserve(strokesArray.size());
        for (const auto &val : strokesArray) {
            page.m_strokes.push_back(Stroke::fromJson(val.toObject()));
        }
    }

    page.m_hasPdfBackground = json["hasPdf"].toBool(false);
    if (page.m_hasPdfBackground && json.contains("pdfPngBase64")) {
        QByteArray bytes = QByteArray::fromBase64(json["pdfPngBase64"].toString().toLatin1());
        page.m_pdfBackground.loadFromData(bytes, "PNG");
    }

    return page;
}

} // namespace Inkwell
