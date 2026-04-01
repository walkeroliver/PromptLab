#pragma once

#include "model/category.h"
#include <QVector>
#include <optional>

class CategoryRepository {
public:
    CategoryRepository();

    int insert(const Category& category);
    bool update(const Category& category);
    bool remove(int id);
    std::optional<Category> findById(int id);
    QVector<Category> findAll();
    QVector<Category> findByParentId(int parentId);
    bool exists(const QString& name);
    bool updateSortOrder(int id, int sortOrder);
    int count();
    int countPrompts(int categoryId);

private:
    Category categoryFromQuery(class QSqlQuery& query);
};
