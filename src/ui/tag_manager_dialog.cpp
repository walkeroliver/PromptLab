#include "tag_manager_dialog.h"
#include "tag_service.h"

#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

TagManagerDialog::TagManagerDialog(TagService* tagService, QWidget* parent)
    : QDialog(parent)
    , m_tagService(tagService)
{
    setupUi();
    loadTags();
}

 
void TagManagerDialog::setupUi()
{
    setWindowTitle(tr("管理标签"));
    setMinimumSize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* contentLayout = new QHBoxLayout;

    m_tagList = new QListWidget;
    contentLayout->addWidget(m_tagList, 2);

    QVBoxLayout* editLayout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText(tr("标签名称..."));
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

    connect(addBtn, &QPushButton::clicked, this, &TagManagerDialog::onAddTag);
    connect(editBtn, &QPushButton::clicked, this, &TagManagerDialog::onEditTag);
    connect(deleteBtn, &QPushButton::clicked, this, &TagManagerDialog::onDeleteTag);

    contentLayout->addLayout(editLayout, 1);
    mainLayout->addLayout(contentLayout);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    buttonBox->button(QDialogButtonBox::Close)->setText(tr("关闭"));
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
    mainLayout->addWidget(buttonBox);

    connect(m_tagList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        m_nameEdit->setText(item->text());
    });
}

 
void TagManagerDialog::loadTags()
{
    m_tagList->clear();

    QVector<Tag> tags = m_tagService->getAllTags();
    for (const Tag& tag : tags) {
        QListWidgetItem* item = new QListWidgetItem(tag.name);
        item->setData(Qt::UserRole, tag.id);
        m_tagList->addItem(item);
    }
}
 
void TagManagerDialog::onAddTag()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("标签名称不能为空。"));
        return;
    }

 
    if (m_tagService->tagExists(name)) {
        QMessageBox::warning(this, tr("错误"), tr("标签已存在。"));
        return;
    }
 
    m_tagService->createTag(name);
    m_nameEdit->clear();
    loadTags();
}
 
void TagManagerDialog::onEditTag()
{
    QListWidgetItem* item = m_tagList->currentItem();
    if (!item) {
        QMessageBox::warning(this, tr("错误"), tr("请选择要编辑的标签。"));
        return;
    }
 
    QString newName = m_nameEdit->text().trimmed();
    if (newName.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("标签名称不能为空。"));
        return;
    }
 
    Tag tag;
    tag.id = item->data(Qt::UserRole).toInt();
    tag.name = newName;
 
    m_tagService->updateTag(tag);
    loadTags();
}
 
void TagManagerDialog::onDeleteTag()
{
    QListWidgetItem* item = m_tagList->currentItem();
    if (!item) {
        QMessageBox::warning(this, tr("错误"), tr("请选择要删除的标签。"));
        return;
    }
 
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("确认删除"),
        tr("确定要删除这个标签吗？"),
        QMessageBox::Yes | QMessageBox::No);
 
    if (reply == QMessageBox::Yes) {
        int id = item->data(Qt::UserRole).toInt();
        m_tagService->deleteTag(id);
        m_nameEdit->clear();
        loadTags();
    }
}
