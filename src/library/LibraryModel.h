#pragma once

#include "NotebookManager.h"
#include <QAbstractListModel>
#include <QJsonArray>
#include <QJsonObject>

namespace Inkwell {

class LibraryModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString currentFolderId READ currentFolderId WRITE setCurrentFolderId NOTIFY currentFolderIdChanged)
    
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        CoverColorRole,
        ModifiedAtRole,
        TypeRole  // "notebook" or "folder"
    };
    
    explicit LibraryModel(NotebookManager *manager, QObject *parent = nullptr);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    QString currentFolderId() const { return m_currentFolderId; }
    void setCurrentFolderId(const QString &id);
    
    Q_INVOKABLE void refresh();

signals:
    void currentFolderIdChanged();

private:
    NotebookManager *m_manager;
    QString m_currentFolderId;
    QJsonArray m_items;
};

} // namespace Inkwell
