#include "NotebookManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDebug>

namespace Inkwell {

NotebookManager::NotebookManager(Database *db, QObject *parent)
    : QObject(parent)
    , m_db(db)
{
}

QString NotebookManager::createNotebook(const QString &title, const QString &folderId)
{
    Notebook notebook;
    notebook.setTitle(title);
    notebook.setFolderId(folderId);
    
    // Generate a random cover color from a palette
    static const QStringList coverColors = {
        "#4A90D9", "#E74C3C", "#2ECC71", "#F39C12",
        "#9B59B6", "#1ABC9C", "#E67E22", "#3498DB",
        "#E91E63", "#00BCD4", "#8BC34A", "#FF9800"
    };
    int colorIndex = QRandomGenerator::global()->bounded(coverColors.size());
    notebook.setCoverColor(QColor(coverColors[colorIndex]));
    
    // Add one blank page
    Page blankPage;
    notebook.addPage(blankPage);
    
    if (!m_db->saveNotebook(notebook)) {
        qWarning() << "Failed to save new notebook";
        return QString();
    }
    
    emit notebooksChanged();
    return notebook.id();
}

bool NotebookManager::saveNotebook(const QString &notebookJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(notebookJson.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid notebook JSON";
        return false;
    }
    
    Notebook notebook = Notebook::fromJson(doc.object());
    notebook.setModifiedAt(QDateTime::currentDateTime());
    
    bool result = m_db->saveNotebook(notebook);
    if (result) {
        emit notebooksChanged();
    }
    return result;
}

bool NotebookManager::deleteNotebook(const QString &id)
{
    bool result = m_db->deleteNotebook(id);
    if (result) {
        emit notebooksChanged();
    }
    return result;
}

QString NotebookManager::loadNotebook(const QString &id)
{
    Notebook notebook = m_db->loadNotebook(id);
    if (notebook.id().isEmpty()) {
        return QString();
    }
    
    QJsonObject json = notebook.toJson();
    QJsonDocument doc(json);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QJsonArray NotebookManager::listNotebooks()
{
    auto notebooks = m_db->listNotebooks();
    QJsonArray array;
    for (const auto &obj : notebooks) {
        array.append(obj);
    }
    return array;
}

QString NotebookManager::createFolder(const QString &name, const QString &parentId)
{
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    if (!m_db->createFolder(id, name, parentId)) {
        qWarning() << "Failed to create folder";
        return QString();
    }
    
    emit foldersChanged();
    return id;
}

bool NotebookManager::deleteFolder(const QString &id)
{
    bool result = m_db->deleteFolder(id);
    if (result) {
        emit foldersChanged();
    }
    return result;
}

bool NotebookManager::renameFolder(const QString &id, const QString &name)
{
    bool result = m_db->renameFolder(id, name);
    if (result) {
        emit foldersChanged();
    }
    return result;
}

QJsonArray NotebookManager::listFolders(const QString &parentId)
{
    auto folders = m_db->listFolders(parentId);
    QJsonArray array;
    for (const auto &obj : folders) {
        array.append(obj);
    }
    return array;
}

} // namespace Inkwell
