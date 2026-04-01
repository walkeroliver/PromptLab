#include "category_service.h"

CategoryService::CategoryService(QObject* parent)
    : QObject(parent)
{
}

int CategoryService::createCategory(const QString& name, int parentId)
{
    Category category;
    category.name = name;
    category.parentId = parentId;

    int id = m_categoryRepo.insert(category);
    if (id > 0) {
        emit categoryCreated(id);
        emit dataChanged();
    }

    return id;
}

bool CategoryService::updateCategory(const Category& category)
{
    bool success = m_categoryRepo.update(category);
    if (success) {
        emit categoryUpdated(category.id);
        emit dataChanged();
    }

    return success;
}

bool CategoryService::deleteCategory(int id)
{
    bool success = m_categoryRepo.remove(id);
    if (success) {
        emit categoryDeleted(id);
        emit dataChanged();
    }

    return success;
}

Category CategoryService::getCategory(int id)
{
    auto category = m_categoryRepo.findById(id);
    return category.value_or(Category());
}

QVector<Category> CategoryService::getAllCategories()
{
    return m_categoryRepo.findAll();
}

QVector<Category> CategoryService::getTopLevelCategories()
{
    return m_categoryRepo.findByParentId(0);
}

QVector<Category> CategoryService::getSubCategories(int parentId)
{
    return m_categoryRepo.findByParentId(parentId);
}

bool CategoryService::categoryExists(const QString& name)
{
    return m_categoryRepo.exists(name);
}
