#include "prompt_editor_dialog.h"
#include "star_rating.h"
#include "prompt_service.h"
#include "category_service.h"
#include "tag_service.h"
#include "tag_manager_dialog.h"

#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QGroupBox>

PromptEditorDialog::PromptEditorDialog(PromptService* promptService,
                                           CategoryService* categoryService,
                                           TagService* tagService,
                                           QWidget* parent)
    : QDialog(parent)
    , m_promptService(promptService)
    , m_categoryService(categoryService)
    , m_tagService(tagService)
{
    setupUi();
    loadCategories();
    loadTags();
}

void PromptEditorDialog::setupUi()
{
    setWindowTitle(tr("新建提示词"));
    setMinimumSize(600, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* formLayout = new QFormLayout;

    m_titleEdit = new QLineEdit;
    m_titleEdit->setPlaceholderText(tr("请输入提示词标题..."));
    formLayout->addRow(tr("标题："), m_titleEdit);

    m_categoryCombo = new QComboBox;
    formLayout->addRow(tr("分类："), m_categoryCombo);

    mainLayout->addLayout(formLayout);

    QGroupBox* contentGroup = new QGroupBox(tr("提示词内容"));
    QVBoxLayout* contentLayout = new QVBoxLayout(contentGroup);
    m_contentEdit = new QTextEdit;
    m_contentEdit->setPlaceholderText(tr("请输入提示词内容..."));
    m_contentEdit->setMinimumHeight(150);
    contentLayout->addWidget(m_contentEdit);
    mainLayout->addWidget(contentGroup);

    QGroupBox* descGroup = new QGroupBox(tr("描述（可选）"));
    QVBoxLayout* descLayout = new QVBoxLayout(descGroup);
    m_descriptionEdit = new QTextEdit;
    m_descriptionEdit->setPlaceholderText(tr("请输入描述信息..."));
    m_descriptionEdit->setMaximumHeight(80);
    descLayout->addWidget(m_descriptionEdit);
    mainLayout->addWidget(descGroup);

    QHBoxLayout* tagLayout = new QHBoxLayout;
    QGroupBox* tagGroup = new QGroupBox(tr("标签"));
    QVBoxLayout* tagBoxLayout = new QVBoxLayout(tagGroup);
    m_tagList = new QListWidget;
    m_tagList->setSelectionMode(QAbstractItemView::MultiSelection);
    tagBoxLayout->addWidget(m_tagList);

    QPushButton* manageTagsBtn = new QPushButton(tr("管理标签..."));
    connect(manageTagsBtn, &QPushButton::clicked, this, &PromptEditorDialog::onManageTags);
    tagBoxLayout->addWidget(manageTagsBtn);

    tagLayout->addWidget(tagGroup);

    QGroupBox* ratingGroup = new QGroupBox(tr("评分"));
    QVBoxLayout* ratingLayout = new QVBoxLayout(ratingGroup);
    m_ratingWidget = new StarRating;
    ratingLayout->addWidget(m_ratingWidget);
    tagLayout->addWidget(ratingGroup);

    mainLayout->addLayout(tagLayout);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Save)->setText(tr("保存"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("取消"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PromptEditorDialog::onSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void PromptEditorDialog::loadCategories()
{
    m_categoryCombo->clear();
    m_categoryCombo->addItem(tr("未分类"), 0);

    QVector<Category> categories = m_categoryService->getAllCategories();
    for (const Category& cat : categories) {
        m_categoryCombo->addItem(cat.name, cat.id);
    }
}

void PromptEditorDialog::loadTags()
{
    m_tagList->clear();

    QVector<Tag> tags = m_tagService->getAllTags();
    for (const Tag& tag : tags) {
        QListWidgetItem* item = new QListWidgetItem(tag.name);
        item->setData(Qt::UserRole, tag.id);
        item->setCheckState(Qt::Unchecked);
        m_tagList->addItem(item);
    }
}

void PromptEditorDialog::setPrompt(const Prompt& prompt)
{
    m_promptId = prompt.id;
    m_isEditMode = true;
    setWindowTitle(tr("编辑提示词"));

    m_titleEdit->setText(prompt.title);
    m_contentEdit->setPlainText(prompt.content);
    m_descriptionEdit->setPlainText(prompt.description);
    m_ratingWidget->setRating(prompt.overallRating);

    for (int i = 0; i < m_categoryCombo->count(); ++i) {
        if (m_categoryCombo->itemData(i).toInt() == prompt.categoryId) {
            m_categoryCombo->setCurrentIndex(i);
            break;
        }
    }

    for (int i = 0; i < m_tagList->count(); ++i) {
        QListWidgetItem* item = m_tagList->item(i);
        int tagId = item->data(Qt::UserRole).toInt();
        if (prompt.tagIds.contains(tagId)) {
            item->setCheckState(Qt::Checked);
        }
    }
}

Prompt PromptEditorDialog::getPrompt() const
{
    Prompt prompt;
    prompt.id = m_promptId;
    prompt.title = m_titleEdit->text().trimmed();
    prompt.content = m_contentEdit->toPlainText();
    prompt.description = m_descriptionEdit->toPlainText();
    prompt.categoryId = m_categoryCombo->currentData().toInt();
    prompt.overallRating = m_ratingWidget->rating();

    for (int i = 0; i < m_tagList->count(); ++i) {
        QListWidgetItem* item = m_tagList->item(i);
        if (item->checkState() == Qt::Checked) {
            prompt.tagIds.append(item->data(Qt::UserRole).toInt());
        }
    }

    return prompt;
}

void PromptEditorDialog::onSave()
{
    if (!validateInput()) {
        return;
    }

    Prompt prompt = getPrompt();

    if (m_isEditMode) {
        m_promptService->updatePrompt(prompt);
    } else {
        m_promptService->createPrompt(prompt);
    }

    accept();
}

bool PromptEditorDialog::validateInput()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("验证错误"), tr("标题为必填项。"));
        return false;
    }

    if (m_contentEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("验证错误"), tr("内容为必填项。"));
        return false;
    }

    return true;
}

void PromptEditorDialog::onManageTags()
{
    TagManagerDialog dialog(m_tagService, this);
    dialog.exec();
    loadTags();
}
