#include "prompt_list_widget.h"
#include "prompt_service.h"
#include "category_service.h"
#include "tag_service.h"
#include "star_rating.h"

#include <QListWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QFrame>
#include <QtMath>

PromptListWidget::PromptListWidget(PromptService* promptService,
                                     CategoryService* categoryService,
                                     TagService* tagService,
                                     QWidget* parent)
    : QWidget(parent)
    , m_promptService(promptService)
    , m_categoryService(categoryService)
    , m_tagService(tagService)
{
    setupUi();
    loadPrompts();
}

void PromptListWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    QHBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* countLabel = new QLabel(tr("提示词"));
    countLabel->setStyleSheet("font-weight: 600; color: #1a1a1a; font-size: 14px;");
    headerLayout->addWidget(countLabel);

    headerLayout->addStretch();

    m_sortCombo = new QComboBox;
    m_sortCombo->addItem(tr("按评分排序"), 0);
    m_sortCombo->addItem(tr("按更新时间"), 1);
    m_sortCombo->addItem(tr("按创建时间"), 2);
    m_sortCombo->addItem(tr("按标题"), 3);
    m_sortCombo->setFixedWidth(130);
    m_sortCombo->setStyleSheet(
        "QComboBox { background: #fff; border: 1px solid #d9d9d9; border-radius: 4px; padding: 4px 8px; }"
    "QComboBox:hover { border-color: #40a9ff; }"
        "QComboBox::drop-down { border: 1px solid #d9d9d9; }"
    );
    headerLayout->addWidget(m_sortCombo);

    mainLayout->addLayout(headerLayout);

    m_listWidget = new QListWidget;
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSpacing(6);
    m_listWidget->setStyleSheet(
        "QListWidget { background: transparent; border: none; }"
        "QListWidget::item { background: #fff; border: 1px solid #e8e8e8; border-radius: 6px; margin: 2px 0; }"
        "QListWidget::item:hover { background: #fafafa; border-color: #d9d9d9; }"
        "QListWidget::item:selected { background: #e6f7ff; border-color: #1890ff; }"
    );
    mainLayout->addWidget(m_listWidget);

    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PromptListWidget::onSortChanged);
    connect(m_listWidget, &QListWidget::itemClicked, this, &PromptListWidget::onItemClicked);
    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &PromptListWidget::onItemDoubleClicked);
}

void PromptListWidget::loadPrompts()
{
    refreshList();
}

void PromptListWidget::refreshList()
{
    m_categoryNames.clear();
    const QVector<Category> categories = m_categoryService->getAllCategories();
    for (const Category& category : categories) {
        m_categoryNames.insert(category.id, category.name);
    }

    m_tagNames.clear();
    const QVector<Tag> tags = m_tagService->getAllTags();
    for (const Tag& tag : tags) {
        m_tagNames.insert(tag.id, tag.name);
    }

    m_prompts = m_promptService->filterPrompts(m_searchKeyword, m_filterCategoryId, m_filterTagId, m_minRating, false);
    updateList();
}

void PromptListWidget::updateList()
{
    m_listWidget->clear();

    for (const Prompt& prompt : m_prompts) {
        QListWidgetItem* item = new QListWidgetItem(m_listWidget);
        item->setData(Qt::UserRole, prompt.id);
        item->setSizeHint(QSize(0, 64));

        QWidget* card = createPromptCard(prompt);
        m_listWidget->setItemWidget(item, card);
    }

    QLabel* countLabel = findChild<QLabel*>();
    if (countLabel) {
        countLabel->setText(tr("共 %1 条提示词").arg(m_prompts.size()));
    }
}

QWidget* PromptListWidget::createPromptCard(const Prompt& prompt)
{
    QWidget* card = new QWidget;
    card->setStyleSheet("background: transparent;");

    QHBoxLayout* layout = new QHBoxLayout(card);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(12);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setSpacing(6);

    QHBoxLayout* titleLayout = new QHBoxLayout;
    titleLayout->setSpacing(8);

    QLabel* titleLabel = new QLabel(prompt.title);
    titleLabel->setStyleSheet("font-weight: 600; color: #1a1a1a; font-size: 14px;");
    titleLabel->setWordWrap(false);
    titleLayout->addWidget(titleLabel, 1);

    leftLayout->addLayout(titleLayout);

    QHBoxLayout* metaLayout = new QHBoxLayout;
    metaLayout->setSpacing(10);

    const QString ratingText = StarRatingText::toDisplayString(prompt.overallRating);
    QLabel* ratingLabel = new QLabel(ratingText);
    ratingLabel->setStyleSheet("color: #f59e0b; font-size: 12px;");
    metaLayout->addWidget(ratingLabel);

    if (prompt.categoryId > 0) {
        const QString categoryName = m_categoryNames.value(prompt.categoryId);
        if (!categoryName.isEmpty()) {
            QLabel* catLabel = new QLabel(categoryName);
            catLabel->setStyleSheet("color: #1890ff; font-size: 11px; background: #e6f7ff; padding: 2px 8px; border-radius: 4px;");
            metaLayout->addWidget(catLabel);
        }
    }

    for (int tagId : prompt.tagIds) {
        const QString tagName = m_tagNames.value(tagId);
        if (!tagName.isEmpty()) {
            QLabel* tagLabel = new QLabel(tagName);
            tagLabel->setStyleSheet("color: #52c41a; font-size: 11px; background: #f6ffed; padding: 2px 8px; border-radius: 4px;");
            metaLayout->addWidget(tagLabel);
            if (metaLayout->count() > 4) break;
        }
    }

    metaLayout->addStretch();

    QString timeText = prompt.updatedAt.toString("MM-dd hh:mm");
    QLabel* timeLabel = new QLabel(timeText);
    timeLabel->setStyleSheet("color: #999; font-size: 11px;");
    metaLayout->addWidget(timeLabel);

    leftLayout->addLayout(metaLayout);

    layout->addLayout(leftLayout, 1);

    return card;
}

void PromptListWidget::setSearchFilter(const QString& keyword)
{
    m_searchKeyword = keyword;
    refreshList();
}

void PromptListWidget::setCategoryFilter(int categoryId)
{
    m_filterCategoryId = categoryId;
    refreshList();
}

void PromptListWidget::setTagFilter(int tagId)
{
    m_filterTagId = tagId;
    refreshList();
}

void PromptListWidget::setRatingFilter(double minRating)
{
    m_minRating = minRating;
    refreshList();
}

void PromptListWidget::clearFilters()
{
    m_searchKeyword.clear();
    m_filterCategoryId = -1;
    m_filterTagId = -1;
    m_minRating = 0.0;
    refreshList();
}

void PromptListWidget::onItemClicked(QListWidgetItem* item)
{
    int promptId = item->data(Qt::UserRole).toInt();
    emit promptSelected(promptId);
}

void PromptListWidget::onItemDoubleClicked(QListWidgetItem* item)
{
    int promptId = item->data(Qt::UserRole).toInt();
    emit promptDoubleClicked(promptId);
}

void PromptListWidget::onSortChanged(int index)
{
    m_sortMode = index;
    refreshList();
}
