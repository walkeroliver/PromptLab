#include "tag_repository.h"
#include "database/database_manager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

TagRepository::TagRepository() {}

int TagRepository::insert(const Tag& tag)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT INTO tags (name, color) VALUES (?, ?)");
    query.addBindValue(tag.name);
    query.addBindValue(tag.color.isEmpty() ? "#3498db" : tag.color);

    if (!query.exec()) {
        qDebug() << "Failed to insert tag:" << query.lastError().text();
        return 0;
    }

    return query.lastInsertId().toInt();
}

bool TagRepository::update(const Tag& tag)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE tags SET name = ?, color = ? WHERE id = ?");
    query.addBindValue(tag.name);
    query.addBindValue(tag.color);
    query.addBindValue(tag.id);

    if (!query.exec()) {
        qDebug() << "Failed to update tag:" << query.lastError().text();
        return false;
    }

    return true;
}

bool TagRepository::remove(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM tags WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qDebug() << "Failed to delete tag:" << query.lastError().text();
        return false;
    }

    return true;
}

std::optional<Tag> TagRepository::findById(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM tags WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        return tagFromQuery(query);
    }

    return std::nullopt;
}

std::optional<Tag> TagRepository::findByName(const QString& name)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM tags WHERE name = ?");
    query.addBindValue(name);

    if (query.exec() && query.next()) {
        return tagFromQuery(query);
    }

    return std::nullopt;
}

QVector<Tag> TagRepository::findAll()
{
    QVector<Tag> tags;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT * FROM tags ORDER BY name");

    while (query.next()) {
        tags.append(tagFromQuery(query));
    }

    return tags;
}

QVector<Tag> TagRepository::findByPromptId(int promptId)
{
    QVector<Tag> tags;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        SELECT t.* FROM tags t
        JOIN prompt_tags pt ON t.id = pt.tag_id
        WHERE pt.prompt_id = ?
        ORDER BY t.name
    )");
    query.addBindValue(promptId);

    if (query.exec()) {
        while (query.next()) {
            tags.append(tagFromQuery(query));
        }
    }

    return tags;
}

QVector<Tag> TagRepository::findPopular(int limit)
{
    QVector<Tag> tags;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM tags ORDER BY usage_count DESC LIMIT ?");
    query.addBindValue(limit);

    if (query.exec()) {
        while (query.next()) {
            tags.append(tagFromQuery(query));
        }
    }

    return tags;
}

bool TagRepository::exists(const QString& name)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT COUNT(*) FROM tags WHERE name = ?");
    query.addBindValue(name);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

int TagRepository::getOrCreate(const QString& name)
{
    auto existing = findByName(name);
    if (existing) {
        return existing->id;
    }

    return insert(Tag{name});
}

int TagRepository::count()
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT COUNT(*) FROM tags");

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

Tag TagRepository::tagFromQuery(QSqlQuery& query)
{
    Tag tag;
    tag.id = query.value("id").toInt();
    tag.name = query.value("name").toString();
    tag.color = query.value("color").toString();
    tag.usageCount = query.value("usage_count").toInt();
    tag.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    return tag;
}
