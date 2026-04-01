#pragma once

#include "model/category.h"
#include "repository/category_repository.h"
#include <QObject>
#include <QVector>

class CategoryService : public QObject {
    Q_OBJECT

public:
    explicit CategoryService(QObject* parent = nullptr);

    int createCategory(const QString& name, int parentId = 0);
    bool updateCategory(const Category& category);
    bool deleteCategory(int id);
    Category getCategory(int id);
    QVector<Category> getAllCategories();
    QVector<Category> getTopLevelCategories();
    QVector<Category> getSubCategories(int parentId);
    bool categoryExists(const QString& name);

signals:
    void categoryCreated(int id);
    void categoryUpdated(int id);
    void categoryDeleted(int id);
    void dataChanged();

private:
    CategoryRepository m_categoryRepo;
};
