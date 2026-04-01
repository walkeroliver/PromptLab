#include "category_manager_dialog.h"
#include "category_service.h"

#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

CategoryManagerDialog::CategoryManagerDialog(CategoryService* categoryService, QWidget* parent)
    : QDialog(parent)
    , m_categoryService(categoryService)
{
    setupUi();
    loadCategories();
}

 
void CategoryManagerDialog::setupUi()
{
    setWindowTitle(tr("管理分类"));
    setMinimumSize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* contentLayout = new QHBoxLayout;

    m_categoryList = new QListWidget;
    contentLayout->addWidget(m_categoryList, 2);

    QVBoxLayout* editLayout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText(tr("分类名称..."));
    formLayout->addRow(tr("名称："), m_nameEdit);
    editLayout->addLayout(formLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* addBtn = new QPushButton(tr("添加"));
    QPushButton* editBtn = new QPushButton(tr("更新"));
    QPushButton* deleteBtn = new QPushButton(tr("删除"));
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(editBtn);
    buttonLayout->addWidget(deleteBtn);
    editLayout->addLayout(buttonLayout);

    connect(addBtn, &QPushButton::clicked, this, &CategoryManagerDialog::onAddCategory);
    connect(editBtn, &QPushButton::clicked, this, &CategoryManagerDialog::onEditCategory);
    connect(deleteBtn, &QPushButton::clicked, this, &CategoryManagerDialog::onDeleteCategory);

    contentLayout->addLayout(editLayout, 1);
    mainLayout->addLayout(contentLayout);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    buttonBox->button(QDialogButtonBox::Close)->setText(tr("关闭"));
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
    mainLayout->addWidget(buttonBox);

    connect(m_categoryList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        m_nameEdit->setText(item->text());
    });
}

 
void CategoryManagerDialog::loadCategories()
{
    m_categoryList->clear();

    QVector<Category> categories = m_categoryService->getAllCategories();
    for (const Category& cat : categories) {
        QListWidgetItem* item = new QListWidgetItem(cat.name);
        item->setData(Qt::UserRole, cat.id);
        m_categoryList->addItem(item);
    }
}

 
void CategoryManagerDialog::onAddCategory()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("分类名称不能为空。"));
        return;
    }

    if (m_categoryService->categoryExists(name)) {
        QMessageBox::warning(this, tr("错误"), tr("分类已存在。"));
        return;
    }
 
    m_categoryService->createCategory(name);
    m_nameEdit->clear();
    loadCategories();
}
 
void CategoryManagerDialog::onEditCategory()
{
    QListWidgetItem* item = m_categoryList->currentItem();
    if (!item) {
        QMessageBox::warning(this, tr("错误"), tr("请选择要编辑的分类。"));
        return;
    }
 
    QString newName = m_nameEdit->text().trimmed();
    if (newName.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("分类名称不能为空。"));
        return;
    }
 
    Category category;
    category.id = item->data(Qt::UserRole).toInt();
    category.name = newName;
 
    m_categoryService->updateCategory(category);
    loadCategories();
}
 
void CategoryManagerDialog::onDeleteCategory()
{
    QListWidgetItem* item = m_categoryList->currentItem();
    if (!item) {
        QMessageBox::warning(this, tr("错误"), tr("请选择要删除的分类。"));
        return;
    }
 
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("确认删除"),
        tr("确定要删除这个分类吗？"),
        QMessageBox::Yes | QMessageBox::No);
 
    if (reply == QMessageBox::Yes) {
        int id = item->data(Qt::UserRole).toInt();
        m_categoryService->deleteCategory(id);
        m_nameEdit->clear();
        loadCategories();
    }
}
