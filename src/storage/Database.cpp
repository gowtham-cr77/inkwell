#include "Database.h"
#include <QDir>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>

namespace Inkwell {

Database::Database(QObject *parent)
    : QObject(parent)
{
}

Database::~Database()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool Database::initialize()
{
    // Create data directory
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        if (!dir.mkpath(dataDir)) {
            qWarning() << "Failed to create data directory:" << dataDir;
            return false;
        }
    }
    
    m_dbPath = dataDir + "/inkwell.db";
    
    // Open SQLite database
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_dbPath);
    
    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    
    // Enable WAL mode for better concurrent performance
    QSqlQuery walQuery(m_db);
    walQuery.exec("PRAGMA journal_mode=WAL");
    walQuery.exec("PRAGMA foreign_keys=ON");
    
    return createTables();
}

bool Database::createTables()
{
    QSqlQuery query(m_db);
    
    // Notebooks table
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS notebooks ("
        "  id TEXT PRIMARY KEY,"
        "  title TEXT NOT NULL,"
        "  cover_color TEXT,"
        "  folder_id TEXT,"
        "  data TEXT,"
        "  created_at TEXT NOT NULL,"
        "  modified_at TEXT NOT NULL"
        ")"
    );
    if (!ok) {
        qWarning() << "Failed to create notebooks table:" << query.lastError().text();
        return false;
    }
    
    // Folders table
    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS folders ("
        "  id TEXT PRIMARY KEY,"
        "  name TEXT NOT NULL,"
        "  parent_id TEXT,"
        "  created_at TEXT NOT NULL"
        ")"
    );
    if (!ok) {
        qWarning() << "Failed to create folders table:" << query.lastError().text();
        return false;
    }
    
    // Settings table
    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS settings ("
        "  key TEXT PRIMARY KEY,"
        "  value TEXT"
        ")"
    );
    if (!ok) {
        qWarning() << "Failed to create settings table:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::saveNotebook(const Notebook &notebook)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT OR REPLACE INTO notebooks (id, title, cover_color, folder_id, data, created_at, modified_at) "
        "VALUES (:id, :title, :cover_color, :folder_id, :data, :created_at, :modified_at)"
    );
    
    QJsonObject json = notebook.toJson();
    QJsonDocument doc(json);
    
    query.bindValue(":id", notebook.id());
    query.bindValue(":title", notebook.title());
    query.bindValue(":cover_color", notebook.coverColor().name());
    query.bindValue(":folder_id", notebook.folderId());
    query.bindValue(":data", QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    query.bindValue(":created_at", notebook.createdAt().toString(Qt::ISODate));
    query.bindValue(":modified_at", notebook.modifiedAt().toString(Qt::ISODate));
    
    if (!query.exec()) {
        qWarning() << "Failed to save notebook:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::deleteNotebook(const QString &id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM notebooks WHERE id = :id");
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete notebook:" << query.lastError().text();
        return false;
    }
    
    return true;
}

Notebook Database::loadNotebook(const QString &id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT data FROM notebooks WHERE id = :id");
    query.bindValue(":id", id);
    
    if (!query.exec() || !query.next()) {
        qWarning() << "Failed to load notebook:" << id;
        return Notebook();
    }
    
    QString dataStr = query.value(0).toString();
    QJsonDocument doc = QJsonDocument::fromJson(dataStr.toUtf8());
    
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid notebook JSON for id:" << id;
        return Notebook();
    }
    
    return Notebook::fromJson(doc.object());
}

std::vector<QJsonObject> Database::listNotebooks()
{
    std::vector<QJsonObject> results;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT id, title, cover_color, folder_id, modified_at FROM notebooks ORDER BY modified_at DESC");
    
    if (!query.exec()) {
        qWarning() << "Failed to list notebooks:" << query.lastError().text();
        return results;
    }
    
    while (query.next()) {
        QJsonObject obj;
        obj["id"] = query.value(0).toString();
        obj["title"] = query.value(1).toString();
        obj["coverColor"] = query.value(2).toString();
        obj["folderId"] = query.value(3).toString();
        obj["modifiedAt"] = query.value(4).toString();
        obj["type"] = "notebook";
        results.push_back(obj);
    }
    
    return results;
}

bool Database::createFolder(const QString &id, const QString &name, const QString &parentId)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO folders (id, name, parent_id, created_at) "
        "VALUES (:id, :name, :parent_id, :created_at)"
    );
    
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":parent_id", parentId);
    query.bindValue(":created_at", QDateTime::currentDateTime().toString(Qt::ISODate));
    
    if (!query.exec()) {
        qWarning() << "Failed to create folder:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::deleteFolder(const QString &id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM folders WHERE id = :id");
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete folder:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::renameFolder(const QString &id, const QString &name)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE folders SET name = :name WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    
    if (!query.exec()) {
        qWarning() << "Failed to rename folder:" << query.lastError().text();
        return false;
    }
    
    return true;
}

std::vector<QJsonObject> Database::listFolders(const QString &parentId)
{
    std::vector<QJsonObject> results;
    
    QSqlQuery query(m_db);
    if (parentId.isEmpty()) {
        query.prepare("SELECT id, name, parent_id, created_at FROM folders WHERE parent_id = '' OR parent_id IS NULL ORDER BY name");
    } else {
        query.prepare("SELECT id, name, parent_id, created_at FROM folders WHERE parent_id = :parent_id ORDER BY name");
        query.bindValue(":parent_id", parentId);
    }
    
    if (!query.exec()) {
        qWarning() << "Failed to list folders:" << query.lastError().text();
        return results;
    }
    
    while (query.next()) {
        QJsonObject obj;
        obj["id"] = query.value(0).toString();
        obj["name"] = query.value(1).toString();
        obj["parentId"] = query.value(2).toString();
        obj["createdAt"] = query.value(3).toString();
        obj["type"] = "folder";
        results.push_back(obj);
    }
    
    return results;
}

bool Database::saveSetting(const QString &key, const QString &value)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (:key, :value)");
    query.bindValue(":key", key);
    query.bindValue(":value", value);
    
    if (!query.exec()) {
        qWarning() << "Failed to save setting:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QString Database::loadSetting(const QString &key, const QString &defaultValue)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT value FROM settings WHERE key = :key");
    query.bindValue(":key", key);
    
    if (!query.exec() || !query.next()) {
        return defaultValue;
    }
    
    return query.value(0).toString();
}

} // namespace Inkwell
