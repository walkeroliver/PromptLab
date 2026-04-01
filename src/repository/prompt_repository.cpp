#include "prompt_repository.h"
#include "database/database_manager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

PromptRepository::PromptRepository() {}

int PromptRepository::insert(const Prompt& prompt)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        INSERT INTO prompts (title, content, description, category_id,
                            overall_rating, is_favorite, is_archived, usage_count)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(prompt.title);
    query.addBindValue(prompt.content);
    query.addBindValue(prompt.description);
    query.addBindValue(prompt.categoryId > 0 ? prompt.categoryId : QVariant());
    query.addBindValue(prompt.overallRating);
    query.addBindValue(prompt.isFavorite ? 1 : 0);
    query.addBindValue(prompt.isArchived ? 1 : 0);
    query.addBindValue(prompt.usageCount);

    if (!query.exec()) {
        qDebug() << "Failed to insert prompt:" << query.lastError().text();
        return 0;
    }

    int promptId = query.lastInsertId().toInt();

    for (int tagId : prompt.tagIds) {
        addTag(promptId, tagId);
    }

    return promptId;
}

bool PromptRepository::update(const Prompt& prompt)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        UPDATE prompts SET
            title = ?,
            content = ?,
            description = ?,
            category_id = ?,
            overall_rating = ?,
            is_favorite = ?,
            is_archived = ?
        WHERE id = ?
    )");

    query.addBindValue(prompt.title);
    query.addBindValue(prompt.content);
    query.addBindValue(prompt.description);
    query.addBindValue(prompt.categoryId > 0 ? prompt.categoryId : QVariant());
    query.addBindValue(prompt.overallRating);
    query.addBindValue(prompt.isFavorite ? 1 : 0);
    query.addBindValue(prompt.isArchived ? 1 : 0);
    query.addBindValue(prompt.id);

    if (!query.exec()) {
        qDebug() << "Failed to update prompt:" << query.lastError().text();
        return false;
    }

    removeAllTags(prompt.id);
    for (int tagId : prompt.tagIds) {
        addTag(prompt.id, tagId);
    }

    return true;
}

bool PromptRepository::remove(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM prompts WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qDebug() << "Failed to delete prompt:" << query.lastError().text();
        return false;
    }

    return true;
}

std::optional<Prompt> PromptRepository::findById(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM prompts WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        Prompt prompt = promptFromQuery(query);
        loadTagsForPrompt(prompt);
        return prompt;
    }

    return std::nullopt;
}

QVector<Prompt> PromptRepository::findAll()
{
    QVector<Prompt> prompts;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT * FROM prompts ORDER BY overall_rating DESC, updated_at DESC");

    while (query.next()) {
        Prompt prompt = promptFromQuery(query);
        loadTagsForPrompt(prompt);
        prompts.append(prompt);
    }

    return prompts;
}

QVector<Prompt> PromptRepository::findAllOrderByRating()
{
    QVector<Prompt> prompts;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT * FROM prompts WHERE is_archived = 0 ORDER BY overall_rating DESC, updated_at DESC");

    while (query.next()) {
        Prompt prompt = promptFromQuery(query);
        loadTagsForPrompt(prompt);
        prompts.append(prompt);
    }

    return prompts;
}

QVector<Prompt> PromptRepository::findAllOrderByUpdated()
{
    QVector<Prompt> prompts;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT * FROM prompts ORDER BY updated_at DESC");

    while (query.next()) {
        Prompt prompt = promptFromQuery(query);
        loadTagsForPrompt(prompt);
        prompts.append(prompt);
    }

    return prompts;
}

QVector<Prompt> PromptRepository::findByCategoryId(int categoryId)
{
    QVector<Prompt> prompts;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM prompts WHERE category_id = ? ORDER BY overall_rating DESC");
    query.addBindValue(categoryId);

    if (query.exec()) {
        while (query.next()) {
            Prompt prompt = promptFromQuery(query);
            loadTagsForPrompt(prompt);
            prompts.append(prompt);
        }
    }

    return prompts;
}

QVector<Prompt> PromptRepository::findByTagId(int tagId)
{
    QVector<Prompt> prompts;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        SELECT p.* FROM prompts p
        JOIN prompt_tags pt ON p.id = pt.prompt_id
        WHERE pt.tag_id = ?
        ORDER BY p.overall_rating DESC
    )");
    query.addBindValue(tagId);

    if (query.exec()) {
        while (query.next()) {
            Prompt prompt = promptFromQuery(query);
            loadTagsForPrompt(prompt);
            prompts.append(prompt);
        }
    }

    return prompts;
}

QVector<Prompt> PromptRepository::search(const QString& keyword)
{
    QVector<Prompt> prompts;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        SELECT * FROM prompts
        WHERE title LIKE ? OR content LIKE ?
        ORDER BY overall_rating DESC
    )");

    QString pattern = "%" + keyword + "%";
    query.addBindValue(pattern);
    query.addBindValue(pattern);

    if (query.exec()) {
        while (query.next()) {
            Prompt prompt = promptFromQuery(query);
            loadTagsForPrompt(prompt);
            prompts.append(prompt);
        }
    }

    return prompts;
}

QVector<Prompt> PromptRepository::searchWithFilter(const QString& keyword, int categoryId,
                                                    int tagId, double minRating, bool favoritesOnly)
{
    QVector<Prompt> prompts;
    QString sql = "SELECT DISTINCT p.* FROM prompts p";
    QStringList conditions;
    QVariantList bindValues;

    if (tagId > 0) {
        sql += " JOIN prompt_tags pt ON p.id = pt.prompt_id";
    }

    if (!keyword.isEmpty()) {
        conditions << "(p.title LIKE ? OR p.content LIKE ?)";
        QString pattern = "%" + keyword + "%";
        bindValues << pattern << pattern;
    }

    if (categoryId > 0) {
        conditions << "p.category_id = ?";
        bindValues << categoryId;
    }

    if (tagId > 0) {
        conditions << "pt.tag_id = ?";
        bindValues << tagId;
    }

    if (minRating > 0) {
        conditions << "p.overall_rating >= ?";
        bindValues << minRating;
    }

    if (favoritesOnly) {
        conditions << "p.is_favorite = 1";
    }

    conditions << "p.is_archived = 0";

    if (!conditions.isEmpty()) {
        sql += " WHERE " + conditions.join(" AND ");
    }

    sql += " ORDER BY p.overall_rating DESC, p.updated_at DESC";

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(sql);

    for (const QVariant& value : bindValues) {
        query.addBindValue(value);
    }

    if (query.exec()) {
        while (query.next()) {
            Prompt prompt = promptFromQuery(query);
            loadTagsForPrompt(prompt);
            prompts.append(prompt);
        }
    }

    return prompts;
}

QVector<Prompt> PromptRepository::findFavorites()
{
    QVector<Prompt> prompts;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT * FROM prompts WHERE is_favorite = 1 ORDER BY updated_at DESC");

    while (query.next()) {
        Prompt prompt = promptFromQuery(query);
        loadTagsForPrompt(prompt);
        prompts.append(prompt);
    }

    return prompts;
}

QVector<Prompt> PromptRepository::findArchived()
{
    QVector<Prompt> prompts;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT * FROM prompts WHERE is_archived = 1 ORDER BY updated_at DESC");

    while (query.next()) {
        Prompt prompt = promptFromQuery(query);
        loadTagsForPrompt(prompt);
        prompts.append(prompt);
    }

    return prompts;
}

int PromptRepository::count()
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT COUNT(*) FROM prompts WHERE is_archived = 0");

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int PromptRepository::countByCategory(int categoryId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT COUNT(*) FROM prompts WHERE category_id = ? AND is_archived = 0");
    query.addBindValue(categoryId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int PromptRepository::countFavorites()
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT COUNT(*) FROM prompts WHERE is_favorite = 1 AND is_archived = 0");

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

bool PromptRepository::addTag(int promptId, int tagId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT OR IGNORE INTO prompt_tags (prompt_id, tag_id) VALUES (?, ?)");
    query.addBindValue(promptId);
    query.addBindValue(tagId);

    return query.exec();
}

bool PromptRepository::removeTag(int promptId, int tagId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM prompt_tags WHERE prompt_id = ? AND tag_id = ?");
    query.addBindValue(promptId);
    query.addBindValue(tagId);

    return query.exec();
}

bool PromptRepository::removeAllTags(int promptId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM prompt_tags WHERE prompt_id = ?");
    query.addBindValue(promptId);

    return query.exec();
}

QVector<int> PromptRepository::findTagIds(int promptId)
{
    QVector<int> tagIds;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT tag_id FROM prompt_tags WHERE prompt_id = ?");
    query.addBindValue(promptId);

    if (query.exec()) {
        while (query.next()) {
            tagIds.append(query.value(0).toInt());
        }
    }

    return tagIds;
}

bool PromptRepository::updateRating(int promptId, double rating)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE prompts SET overall_rating = ? WHERE id = ?");
    query.addBindValue(rating);
    query.addBindValue(promptId);

    return query.exec();
}

bool PromptRepository::setFavorite(int id, bool favorite)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE prompts SET is_favorite = ? WHERE id = ?");
    query.addBindValue(favorite ? 1 : 0);
    query.addBindValue(id);

    return query.exec();
}

bool PromptRepository::setArchive(int id, bool archive)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE prompts SET is_archived = ? WHERE id = ?");
    query.addBindValue(archive ? 1 : 0);
    query.addBindValue(id);

    return query.exec();
}

bool PromptRepository::incrementUsage(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE prompts SET usage_count = usage_count + 1 WHERE id = ?");
    query.addBindValue(id);

    return query.exec();
}

double PromptRepository::getAverageRating()
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT AVG(overall_rating) FROM prompts WHERE is_archived = 0 AND overall_rating > 0");

    if (query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

QVector<Prompt> PromptRepository::findTopRated(int limit)
{
    QVector<Prompt> prompts;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM prompts WHERE is_archived = 0 ORDER BY overall_rating DESC LIMIT ?");
    query.addBindValue(limit);

    if (query.exec()) {
        while (query.next()) {
            Prompt prompt = promptFromQuery(query);
            loadTagsForPrompt(prompt);
            prompts.append(prompt);
        }
    }

    return prompts;
}

QVector<Prompt> PromptRepository::findRecent(int limit)
{
    QVector<Prompt> prompts;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM prompts WHERE is_archived = 0 ORDER BY updated_at DESC LIMIT ?");
    query.addBindValue(limit);

    if (query.exec()) {
        while (query.next()) {
            Prompt prompt = promptFromQuery(query);
            loadTagsForPrompt(prompt);
            prompts.append(prompt);
        }
    }

    return prompts;
}

Prompt PromptRepository::promptFromQuery(QSqlQuery& query)
{
    Prompt prompt;
    prompt.id = query.value("id").toInt();
    prompt.title = query.value("title").toString();
    prompt.content = query.value("content").toString();
    prompt.description = query.value("description").toString();
    prompt.categoryId = query.value("category_id").toInt();
    prompt.overallRating = query.value("overall_rating").toDouble();
    prompt.isFavorite = query.value("is_favorite").toInt() == 1;
    prompt.isArchived = query.value("is_archived").toInt() == 1;
    prompt.usageCount = query.value("usage_count").toInt();
    prompt.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    prompt.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    return prompt;
}

void PromptRepository::loadTagsForPrompt(Prompt& prompt)
{
    prompt.tagIds = findTagIds(prompt.id);
}
