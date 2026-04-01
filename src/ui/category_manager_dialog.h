#pragma once

#include <QDialog>

class CategoryService;
class QListWidget;
class QLineEdit;

class CategoryManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit CategoryManagerDialog(CategoryService* categoryService, QWidget* parent = nullptr);

private slots:
    void onAddCategory();
    void onEditCategory();
    void onDeleteCategory();

private:
    void setupUi();
    void loadCategories();

    CategoryService* m_categoryService;

    QListWidget* m_categoryList;
    QLineEdit* m_nameEdit;
};
