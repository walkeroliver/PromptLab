#include "database_manager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::initialize(const QString& dbPath)
{
    m_dbPath = dbPath;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        m_lastError = "Failed to open database: " + db.lastError().text();
        return false;
    }

    db.exec("PRAGMA foreign_keys = ON");
    db.exec("PRAGMA journal_mode = WAL");
    db.exec("PRAGMA synchronous = NORMAL");

    if (!createTables()) {
        return false;
    }

    if (!upgradeDatabase()) {
        return false;
    }

    if (!createIndexes()) {
        return false;
    }

    if (!createTriggers()) {
        return false;
    }

    if (!insertDefaultData()) {
        return false;
    }

    m_initialized = true;
    qDebug() << "Database initialized successfully:" << dbPath;
    return true;
}

void DatabaseManager::close()
{
    if (m_initialized) {
        QSqlDatabase::database().close();
        m_initialized = false;
    }
}

bool DatabaseManager::isInitialized() const
{
    return m_initialized;
}

QSqlDatabase DatabaseManager::database() const
{
    return QSqlDatabase::database();
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

QString DatabaseManager::databasePath() const
{
    return m_dbPath;
}

bool DatabaseManager::executeSql(const QString& sql)
{
    QSqlQuery query(database());
    if (!query.exec(sql)) {
        m_lastError = "SQL error: " + query.lastError().text();
        qDebug() << m_lastError;
        return false;
    }
    return true;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(database());

    QString createCategories = R"(
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            parent_id INTEGER DEFAULT 0,
            sort_order INTEGER DEFAULT 0,
            created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            updated_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (parent_id) REFERENCES categories(id) ON DELETE SET NULL
        )
    )";
    if (!query.exec(createCategories)) {
        m_lastError = "Failed to create categories table: " + query.lastError().text();
        return false;
    }

    QString createTags = R"(
        CREATE TABLE IF NOT EXISTS tags (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            color TEXT DEFAULT '#3498db',
            usage_count INTEGER DEFAULT 0,
            created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime'))
        )
    )";
    if (!query.exec(createTags)) {
        m_lastError = "Failed to create tags table: " + query.lastError().text();
        return false;
    }

    QString createPrompts = R"(
        CREATE TABLE IF NOT EXISTS prompts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            content TEXT NOT NULL,
            description TEXT,
            category_id INTEGER,
            overall_rating REAL DEFAULT 0.0,
            is_favorite INTEGER DEFAULT 0,
            is_archived INTEGER DEFAULT 0,
            usage_count INTEGER DEFAULT 0,
            search_text TEXT,
            created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            updated_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (category_id) REFERENCES categories(id) ON DELETE SET NULL
        )
    )";
    if (!query.exec(createPrompts)) {
        m_lastError = "Failed to create prompts table: " + query.lastError().text();
        return false;
    }

    QString createPromptTags = R"(
        CREATE TABLE IF NOT EXISTS prompt_tags (
            prompt_id INTEGER NOT NULL,
            tag_id INTEGER NOT NULL,
            created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            PRIMARY KEY (prompt_id, tag_id),
            FOREIGN KEY (prompt_id) REFERENCES prompts(id) ON DELETE CASCADE,
            FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
        )
    )";
    if (!query.exec(createPromptTags)) {
        m_lastError = "Failed to create prompt_tags table: " + query.lastError().text();
        return false;
    }

    QString createModels = R"(
        CREATE TABLE IF NOT EXISTS models (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            provider TEXT,
            is_builtin INTEGER DEFAULT 0,
            sort_order INTEGER DEFAULT 0,
            created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime'))
        )
    )";
    if (!query.exec(createModels)) {
        m_lastError = "Failed to create models table: " + query.lastError().text();
        return false;
    }

    QString createModelScores = R"(
        CREATE TABLE IF NOT EXISTS model_scores (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            prompt_id INTEGER NOT NULL,
            model_id INTEGER NOT NULL,
            model_version TEXT,
            score REAL DEFAULT 0.0,
            note TEXT,
            tested_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            updated_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            UNIQUE (prompt_id, model_id),
            FOREIGN KEY (prompt_id) REFERENCES prompts(id) ON DELETE CASCADE,
            FOREIGN KEY (model_id) REFERENCES models(id) ON DELETE CASCADE
        )
    )";
    if (!query.exec(createModelScores)) {
        m_lastError = "Failed to create model_scores table: " + query.lastError().text();
        return false;
    }

    QString createVersions = R"(
        CREATE TABLE IF NOT EXISTS versions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            prompt_id INTEGER NOT NULL,
            version INTEGER NOT NULL,
            title TEXT NOT NULL,
            content TEXT NOT NULL,
            description TEXT,
            change_note TEXT,
            created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (prompt_id) REFERENCES prompts(id) ON DELETE CASCADE,
            UNIQUE (prompt_id, version)
        )
    )";
    if (!query.exec(createVersions)) {
        m_lastError = "Failed to create versions table: " + query.lastError().text();
        return false;
    }

    QString createExamples = R"(
        CREATE TABLE IF NOT EXISTS examples (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            prompt_id INTEGER NOT NULL,
            input_text TEXT,
            output_text TEXT,
            note TEXT,
            sort_order INTEGER DEFAULT 0,
            created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (prompt_id) REFERENCES prompts(id) ON DELETE CASCADE
        )
    )";
    if (!query.exec(createExamples)) {
        m_lastError = "Failed to create examples table: " + query.lastError().text();
        return false;
    }

    QString createSettings = R"(
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT,
            updated_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime'))
        )
    )";
    if (!query.exec(createSettings)) {
        m_lastError = "Failed to create settings table: " + query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::createIndexes()
{
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_categories_parent_id ON categories(parent_id)",
        "CREATE INDEX IF NOT EXISTS idx_categories_sort_order ON categories(sort_order)",
        "CREATE INDEX IF NOT EXISTS idx_tags_name ON tags(name)",
        "CREATE INDEX IF NOT EXISTS idx_tags_usage_count ON tags(usage_count DESC)",
        "CREATE INDEX IF NOT EXISTS idx_prompts_title ON prompts(title)",
        "CREATE INDEX IF NOT EXISTS idx_prompts_category_id ON prompts(category_id)",
        "CREATE INDEX IF NOT EXISTS idx_prompts_overall_rating ON prompts(overall_rating DESC)",
        "CREATE INDEX IF NOT EXISTS idx_prompts_updated_at ON prompts(updated_at DESC)",
        "CREATE INDEX IF NOT EXISTS idx_prompts_created_at ON prompts(created_at DESC)",
        "CREATE INDEX IF NOT EXISTS idx_prompts_is_favorite ON prompts(is_favorite)",
        "CREATE INDEX IF NOT EXISTS idx_prompts_is_archived ON prompts(is_archived)",
        "CREATE INDEX IF NOT EXISTS idx_prompt_tags_prompt_id ON prompt_tags(prompt_id)",
        "CREATE INDEX IF NOT EXISTS idx_prompt_tags_tag_id ON prompt_tags(tag_id)",
        "CREATE INDEX IF NOT EXISTS idx_models_name ON models(name)",
        "CREATE INDEX IF NOT EXISTS idx_models_sort_order ON models(sort_order)",
        "CREATE INDEX IF NOT EXISTS idx_model_scores_prompt_id ON model_scores(prompt_id)",
        "CREATE INDEX IF NOT EXISTS idx_model_scores_model_id ON model_scores(model_id)",
        "CREATE INDEX IF NOT EXISTS idx_model_scores_score ON model_scores(score DESC)",
        "CREATE INDEX IF NOT EXISTS idx_versions_prompt_id ON versions(prompt_id)",
        "CREATE INDEX IF NOT EXISTS idx_versions_version ON versions(prompt_id, version DESC)",
        "CREATE INDEX IF NOT EXISTS idx_examples_prompt_id ON examples(prompt_id)"
    };

    for (const QString& idx : indexes) {
        if (!executeSql(idx)) {
            return false;
        }
    }

    return true;
}

bool DatabaseManager::createTriggers()
{
    QString trgPromptsUpdate = R"(
        CREATE TRIGGER IF NOT EXISTS trg_prompts_updated_at
        AFTER UPDATE ON prompts
        BEGIN
            UPDATE prompts SET updated_at = datetime('now', 'localtime') WHERE id = NEW.id;
        END
    )";
    if (!executeSql(trgPromptsUpdate)) {
        return false;
    }

    QString trgCategoriesUpdate = R"(
        CREATE TRIGGER IF NOT EXISTS trg_categories_updated_at
        AFTER UPDATE ON categories
        BEGIN
            UPDATE categories SET updated_at = datetime('now', 'localtime') WHERE id = NEW.id;
        END
    )";
    if (!executeSql(trgCategoriesUpdate)) {
        return false;
    }

    QString trgModelScoresUpdate = R"(
        CREATE TRIGGER IF NOT EXISTS trg_model_scores_updated_at
        AFTER UPDATE ON model_scores
        BEGIN
            UPDATE model_scores SET updated_at = datetime('now', 'localtime') WHERE id = NEW.id;
        END
    )";
    if (!executeSql(trgModelScoresUpdate)) {
        return false;
    }

    QString trgSettingsUpdate = R"(
        CREATE TRIGGER IF NOT EXISTS trg_settings_updated_at
        AFTER UPDATE ON settings
        BEGIN
            UPDATE settings SET updated_at = datetime('now', 'localtime') WHERE key = NEW.key;
        END
    )";
    if (!executeSql(trgSettingsUpdate)) {
        return false;
    }

    QString trgTagUsageInsert = R"(
        CREATE TRIGGER IF NOT EXISTS trg_tag_usage_insert
        AFTER INSERT ON prompt_tags
        BEGIN
            UPDATE tags SET usage_count = usage_count + 1 WHERE id = NEW.tag_id;
        END
    )";
    if (!executeSql(trgTagUsageInsert)) {
        return false;
    }

    QString trgTagUsageDelete = R"(
        CREATE TRIGGER IF NOT EXISTS trg_tag_usage_delete
        AFTER DELETE ON prompt_tags
        BEGIN
            UPDATE tags SET usage_count = usage_count - 1 WHERE id = OLD.tag_id;
        END
    )";
    if (!executeSql(trgTagUsageDelete)) {
        return false;
    }

    return true;
}

bool DatabaseManager::insertDefaultData()
{
    QSqlQuery query(database());

    query.exec("SELECT COUNT(*) FROM models");
    if (query.next() && query.value(0).toInt() > 0) {
        return true;
    }

    QStringList defaultModels = {
        "INSERT OR IGNORE INTO models (name, provider, is_builtin, sort_order) VALUES ('GPT', 'OpenAI', 1, 1)",
        "INSERT OR IGNORE INTO models (name, provider, is_builtin, sort_order) VALUES ('Claude', 'Anthropic', 1, 2)",
        "INSERT OR IGNORE INTO models (name, provider, is_builtin, sort_order) VALUES ('Gemini', 'Google', 1, 3)",
        "INSERT OR IGNORE INTO models (name, provider, is_builtin, sort_order) VALUES ('DeepSeek', 'DeepSeek', 1, 4)"
    };

    for (const QString& sql : defaultModels) {
        if (!query.exec(sql)) {
            m_lastError = "Failed to insert default model: " + query.lastError().text();
            return false;
        }
    }

    QStringList defaultSettings = {
        "INSERT OR IGNORE INTO settings (key, value) VALUES ('db_version', '1.0')",
        "INSERT OR IGNORE INTO settings (key, value) VALUES ('theme', 'light')",
        "INSERT OR IGNORE INTO settings (key, value) VALUES ('default_sort', 'rating')"
    };

    for (const QString& sql : defaultSettings) {
        if (!query.exec(sql)) {
            m_lastError = "Failed to insert default setting: " + query.lastError().text();
            return false;
        }
    }

    return true;
}

bool DatabaseManager::upgradeDatabase()
{
    QSqlQuery query(database());

    auto hasColumn = [](QSqlQuery& q, const QString& table, const QString& column) -> bool {
        q.exec(QString("PRAGMA table_info(%1)").arg(table));
        while (q.next()) {
            if (q.value(1).toString() == column) {
                return true;
            }
        }
        return false;
    };

    auto addColumn = [&](const QString& table, const QString& column, const QString& definition) -> bool {
        if (!hasColumn(query, table, column)) {
            QString sql = QString("ALTER TABLE %1 ADD COLUMN %2 %3").arg(table, column, definition);
            if (!query.exec(sql)) {
                m_lastError = QString("Failed to add %1 column to %2 table: %3")
                    .arg(column, table, query.lastError().text());
                return false;
            }
            qDebug() << QString("Added %1 column to %2 table").arg(column, table);
        }
        return true;
    };

    // Tags table columns
    if (!addColumn("tags", "usage_count", "INTEGER DEFAULT 0")) return false;
    if (!addColumn("tags", "color", "TEXT DEFAULT '#3498db'")) return false;
    if (!addColumn("tags", "created_at", "TEXT DEFAULT (datetime('now', 'localtime'))")) return false;

    // Prompts table columns
    if (!addColumn("prompts", "usage_count", "INTEGER DEFAULT 0")) return false;
    if (!addColumn("prompts", "search_text", "TEXT")) return false;
    if (!addColumn("prompts", "created_at", "TEXT DEFAULT (datetime('now', 'localtime'))")) return false;
    if (!addColumn("prompts", "updated_at", "TEXT DEFAULT (datetime('now', 'localtime'))")) return false;

    // Categories table columns
    if (!addColumn("categories", "parent_id", "INTEGER DEFAULT 0")) return false;
    if (!addColumn("categories", "sort_order", "INTEGER DEFAULT 0")) return false;
    if (!addColumn("categories", "created_at", "TEXT DEFAULT (datetime('now', 'localtime'))")) return false;
    if (!addColumn("categories", "updated_at", "TEXT DEFAULT (datetime('now', 'localtime'))")) return false;

    // Models table columns
    if (!addColumn("models", "is_builtin", "INTEGER DEFAULT 0")) return false;
    if (!addColumn("models", "sort_order", "INTEGER DEFAULT 0")) return false;
    if (!addColumn("models", "created_at", "TEXT DEFAULT (datetime('now', 'localtime'))")) return false;
    if (!addColumn("models", "updated_at", "TEXT DEFAULT (datetime('now', 'localtime'))")) return false;

    // Model scores table columns
    if (!addColumn("model_scores", "model_version", "TEXT")) return false;
    if (!addColumn("model_scores", "note", "TEXT")) return false;
    if (!addColumn("model_scores", "created_at", "TEXT DEFAULT (datetime('now', 'localtime'))")) return false;
    if (!addColumn("model_scores", "updated_at", "TEXT DEFAULT (datetime('now', 'localtime'))")) return false;

    // Examples table columns
    if (!addColumn("examples", "sort_order", "INTEGER DEFAULT 0")) return false;
    if (!addColumn("examples", "created_at", "TEXT DEFAULT (datetime('now', 'localtime'))")) return false;

    // Check and update db version
    query.exec("SELECT value FROM settings WHERE key = 'db_version'");
    QString currentVersion = "0.0";
    if (query.next()) {
        currentVersion = query.value(0).toString();
    }

    if (currentVersion < "1.0") {
        query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?)");
        query.bindValue(0, "db_version");
        query.bindValue(1, "1.0");
        if (!query.exec()) {
            m_lastError = "Failed to update database version: " + query.lastError().text();
            return false;
        }
        qDebug() << "Database upgraded to version 1.0";
    }

    return true;
}
