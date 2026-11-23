#include "Notebook.h"

namespace Inkwell {

Notebook::Notebook()
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_title("Untitled Notebook")
    , m_createdAt(QDateTime::currentDateTime())
    , m_modifiedAt(QDateTime::currentDateTime())
{
}

Notebook::Notebook(const QString &title)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_title(title)
    , m_createdAt(QDateTime::currentDateTime())
    , m_modifiedAt(QDateTime::currentDateTime())
{
}

void Notebook::addPage(const Page &page)
{
    m_pages.push_back(page);
    touch();
}

void Notebook::insertPage(int index, const Page &page)
{
    if (index >= 0 && index <= static_cast<int>(m_pages.size())) {
        m_pages.insert(m_pages.begin() + index, page);
        touch();
    }
}

void Notebook::removePage(int index)
{
    if (index >= 0 && index < static_cast<int>(m_pages.size())) {
        m_pages.erase(m_pages.begin() + index);
        touch();
    }
}

void Notebook::movePage(int from, int to)
{
    if (from >= 0 && from < static_cast<int>(m_pages.size()) &&
        to >= 0 && to < static_cast<int>(m_pages.size()) && from != to) {
        Page p = std::move(m_pages[from]);
        m_pages.erase(m_pages.begin() + from);
        m_pages.insert(m_pages.begin() + to, std::move(p));
        touch();
    }
}

Page* Notebook::page(int index)
{
    if (index >= 0 && index < static_cast<int>(m_pages.size())) {
        return &m_pages[index];
    }
    return nullptr;
}

const Page* Notebook::page(int index) const
{
    if (index >= 0 && index < static_cast<int>(m_pages.size())) {
        return &m_pages[index];
    }
    return nullptr;
}

QJsonObject Notebook::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["title"] = m_title;
    obj["coverColor"] = m_coverColor.name(QColor::HexArgb);
    obj["folderId"] = m_folderId;
    obj["createdAt"] = m_createdAt.toString(Qt::ISODate);
    obj["modifiedAt"] = m_modifiedAt.toString(Qt::ISODate);

    QJsonArray pagesArray;
    for (const auto &p : m_pages) {
        pagesArray.append(p.toJson());
    }
    obj["pages"] = pagesArray;

    return obj;
}

Notebook Notebook::fromJson(const QJsonObject &json)
{
    Notebook nb;
    nb.m_id = json["id"].toString(QUuid::createUuid().toString(QUuid::WithoutBraces));
    nb.m_title = json["title"].toString("Untitled Notebook");
    nb.m_coverColor = QColor(json["coverColor"].toString("#4A90D9"));
    nb.m_folderId = json["folderId"].toString();
    nb.m_createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
    nb.m_modifiedAt = QDateTime::fromString(json["modifiedAt"].toString(), Qt::ISODate);

    QJsonArray pagesArray = json["pages"].toArray();
    nb.m_pages.reserve(pagesArray.size());
    for (const auto &val : pagesArray) {
        nb.m_pages.push_back(Page::fromJson(val.toObject()));
    }

    return nb;
}

} // namespace Inkwell
