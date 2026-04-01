#pragma once

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool initialize(const QString& dbPath);
    void close();
    bool isInitialized() const;
    QSqlDatabase database() const;
    QString lastError() const;
    QString databasePath() const;
    bool executeSql(const QString& sql);

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTables();
    bool createIndexes();
    bool createTriggers();
    bool insertDefaultData();
    bool upgradeDatabase();

    QString m_dbPath;
    QString m_lastError;
    bool m_initialized = false;
    static constexpr const char* DB_VERSION = "1.0";
};
