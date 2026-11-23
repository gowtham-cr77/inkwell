#include "LibraryModel.h"
#include <QJsonObject>
#include <algorithm>

namespace Inkwell {

LibraryModel::LibraryModel(NotebookManager *manager, QObject *parent)
    : QAbstractListModel(parent)
    , m_manager(manager)
{
    connect(m_manager, &NotebookManager::notebooksChanged, this, &LibraryModel::refresh);
    connect(m_manager, &NotebookManager::foldersChanged, this, &LibraryModel::refresh);
    
    refresh();
}

int LibraryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();
    
    const QJsonObject item = m_items[index.row()].toObject();
    
    switch (role) {
    case IdRole:
        return item["id"].toString();
    case TitleRole: {
        QString type = item["type"].toString();
        if (type == "folder")
            return item["name"].toString();
        return item["title"].toString();
    }
    case CoverColorRole:
        return item["coverColor"].toString();
    case ModifiedAtRole:
        return item["modifiedAt"].toString();
    case TypeRole:
        return item["type"].toString();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> LibraryModel::roleNames() const
{
    return {
        {IdRole, "itemId"},
        {TitleRole, "title"},
        {CoverColorRole, "coverColor"},
        {ModifiedAtRole, "modifiedAt"},
        {TypeRole, "itemType"}
    };
}

void LibraryModel::setCurrentFolderId(const QString &id)
{
    if (m_currentFolderId != id) {
        m_currentFolderId = id;
        emit currentFolderIdChanged();
        refresh();
    }
}

void LibraryModel::refresh()
{
    beginResetModel();
    
    m_items = QJsonArray();
    
    // Load folders first
    QJsonArray folders = m_manager->listFolders(m_currentFolderId);
    for (const auto &folder : folders) {
        m_items.append(folder);
    }
    
    // Then load notebooks (already sorted by modified date from DB)
    QJsonArray notebooks = m_manager->listNotebooks();
    for (const auto &nb : notebooks) {
        QJsonObject obj = nb.toObject();
        // Filter by current folder
        QString nbFolderId = obj["folderId"].toString();
        if (nbFolderId == m_currentFolderId) {
            m_items.append(obj);
        }
    }
    
    endResetModel();
}

} // namespace Inkwell
