#pragma once

#include "Page.h"
#include <QObject>
#include <QJsonObject>
#include <QDateTime>
#include <vector>
#include <memory>

namespace Inkwell {

class Notebook {
public:
    Notebook();
    explicit Notebook(const QString &title);

    // Identity
    QString id() const { return m_id; }

    // Metadata
    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    QColor coverColor() const { return m_coverColor; }
    void setCoverColor(const QColor &color) { m_coverColor = color; }

    QString folderId() const { return m_folderId; }
    void setFolderId(const QString &folderId) { m_folderId = folderId; }

    QDateTime createdAt() const { return m_createdAt; }
    QDateTime modifiedAt() const { return m_modifiedAt; }
    void touch() { m_modifiedAt = QDateTime::currentDateTime(); }

    // Pages
    void addPage(const Page &page);
    void insertPage(int index, const Page &page);
    void removePage(int index);
    void movePage(int from, int to);
    Page* page(int index);
    const Page* page(int index) const;
    int pageCount() const { return static_cast<int>(m_pages.size()); }
    const std::vector<Page>& pages() const { return m_pages; }
    std::vector<Page>& pages() { return m_pages; }

    // Serialization
    QJsonObject toJson() const;
    static Notebook fromJson(const QJsonObject &json);

private:
    QString m_id;
    QString m_title;
    QColor m_coverColor = QColor("#4A90D9");
    QString m_folderId;
    QDateTime m_createdAt;
    QDateTime m_modifiedAt;
    std::vector<Page> m_pages;
};

} // namespace Inkwell
