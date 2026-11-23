#pragma once

#include "../core/Notebook.h"
#include "../storage/Database.h"
#include <QObject>
#include <QString>
#include <QJsonArray>

namespace Inkwell {

class NotebookManager : public QObject {
    Q_OBJECT
public:
    explicit NotebookManager(Database *db, QObject *parent = nullptr);
    
    Q_INVOKABLE QString createNotebook(const QString &title, const QString &folderId = "");
    Q_INVOKABLE bool saveNotebook(const QString &notebookJson);
    Q_INVOKABLE bool deleteNotebook(const QString &id);
    Q_INVOKABLE QString loadNotebook(const QString &id);
    Q_INVOKABLE QJsonArray listNotebooks();
    
    Q_INVOKABLE QString createFolder(const QString &name, const QString &parentId = "");
    Q_INVOKABLE bool deleteFolder(const QString &id);
    Q_INVOKABLE bool renameFolder(const QString &id, const QString &name);
    Q_INVOKABLE QJsonArray listFolders(const QString &parentId = "");
    
signals:
    void notebooksChanged();
    void foldersChanged();

private:
    Database *m_db;
};

} // namespace Inkwell
