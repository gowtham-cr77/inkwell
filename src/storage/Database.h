#pragma once

#include "../core/Notebook.h"
#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QJsonObject>
#include <vector>

namespace Inkwell {

class Database : public QObject {
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();
    
    bool initialize();
    
    // Notebooks
    bool saveNotebook(const Notebook &notebook);
    bool deleteNotebook(const QString &id);
    Notebook loadNotebook(const QString &id);
    std::vector<QJsonObject> listNotebooks(); // returns metadata only
    
    // Folders
    bool createFolder(const QString &id, const QString &name, const QString &parentId = "");
    bool deleteFolder(const QString &id);
    bool renameFolder(const QString &id, const QString &name);
    std::vector<QJsonObject> listFolders(const QString &parentId = "");
    
    // Settings
    bool saveSetting(const QString &key, const QString &value);
    QString loadSetting(const QString &key, const QString &defaultValue = "");

private:
    bool createTables();
    QSqlDatabase m_db;
    QString m_dbPath;
};

} // namespace Inkwell
