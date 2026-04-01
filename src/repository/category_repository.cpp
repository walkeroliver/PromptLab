#include "category_repository.h"
#include "database/database_manager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

CategoryRepository::CategoryRepository() {}

int CategoryRepository::insert(const Category& category)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT INTO categories (name, parent_id, sort_order) VALUES (?, ?, ?)");
    query.addBindValue(category.name);
    query.addBindValue(category.parentId > 0 ? category.parentId : 0);
    query.addBindValue(category.sortOrder);

    if (!query.exec()) {
        qDebug() << "Failed to insert category:" << query.lastError().text();
        return 0;
    }

    return query.lastInsertId().toInt();
}

bool CategoryRepository::update(const Category& category)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE categories SET name = ?, parent_id = ?, sort_order = ? WHERE id = ?");
    query.addBindValue(category.name);
    query.addBindValue(category.parentId > 0 ? category.parentId : 0);
    query.addBindValue(category.sortOrder);
    query.addBindValue(category.id);

    if (!query.exec()) {
        qDebug() << "Failed to update category:" << query.lastError().text();
        return false;
    }

    return true;
}

bool CategoryRepository::remove(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM categories WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qDebug() << "Failed to delete category:" << query.lastError().text();
        return false;
    }

    return true;
}

std::optional<Category> CategoryRepository::findById(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM categories WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        return categoryFromQuery(query);
    }

    return std::nullopt;
}

QVector<Category> CategoryRepository::findAll()
{
    QVector<Category> categories;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT * FROM categories ORDER BY sort_order, name");

    while (query.next()) {
        categories.append(categoryFromQuery(query));
    }

    return categories;
}

QVector<Category> CategoryRepository::findByParentId(int parentId)
{
    QVector<Category> categories;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM categories WHERE parent_id = ? ORDER BY sort_order, name");
    query.addBindValue(parentId);

    if (query.exec()) {
        while (query.next()) {
            categories.append(categoryFromQuery(query));
        }
    }

    return categories;
}

bool CategoryRepository::exists(const QString& name)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT COUNT(*) FROM categories WHERE name = ?");
    query.addBindValue(name);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

bool CategoryRepository::updateSortOrder(int id, int sortOrder)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE categories SET sort_order = ? WHERE id = ?");
    query.addBindValue(sortOrder);
    query.addBindValue(id);

    return query.exec();
}

int CategoryRepository::count()
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT COUNT(*) FROM categories");

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int CategoryRepository::countPrompts(int categoryId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT COUNT(*) FROM prompts WHERE category_id = ?");
    query.addBindValue(categoryId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

Category CategoryRepository::categoryFromQuery(QSqlQuery& query)
{
    Category category;
    category.id = query.value("id").toInt();
    category.name = query.value("name").toString();
    category.parentId = query.value("parent_id").toInt();
    category.sortOrder = query.value("sort_order").toInt();
    category.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    category.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    return category;
}
