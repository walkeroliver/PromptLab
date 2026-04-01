#include "dashboard_widget.h"
#include "prompt_service.h"
#include "category_service.h"
#include "tag_service.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>

DashboardWidget::DashboardWidget(PromptService* promptService,
                                   CategoryService* categoryService,
                                   TagService* tagService,
                                   QWidget* parent)
    : QWidget(parent)
    , m_promptService(promptService)
    , m_categoryService(categoryService)
    , m_tagService(tagService)
{
    setupUi();
    updateStats();
}

void DashboardWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel* titleLabel = new QLabel(tr("仪表盘"));
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(20);

    QGridLayout* statsGrid = new QGridLayout;

    QGroupBox* promptsGroup = new QGroupBox(tr("提示词"));
    QVBoxLayout* promptsLayout = new QVBoxLayout(promptsGroup);
    m_totalPromptsLabel = new QLabel;
    m_totalPromptsLabel->setStyleSheet("font-size: 32px; font-weight: bold;");
    m_totalPromptsLabel->setAlignment(Qt::AlignCenter);
    promptsLayout->addWidget(m_totalPromptsLabel);
    promptsLayout->addWidget(new QLabel(tr("提示词总数")));
    statsGrid->addWidget(promptsGroup, 0, 0);

    QGroupBox* categoriesGroup = new QGroupBox(tr("分类"));
    QVBoxLayout* categoriesLayout = new QVBoxLayout(categoriesGroup);
    m_totalCategoriesLabel = new QLabel;
    m_totalCategoriesLabel->setStyleSheet("font-size: 32px; font-weight: bold;");
    m_totalCategoriesLabel->setAlignment(Qt::AlignCenter);
    categoriesLayout->addWidget(m_totalCategoriesLabel);
    categoriesLayout->addWidget(new QLabel(tr("分类总数")));
    statsGrid->addWidget(categoriesGroup, 0, 1);

    QGroupBox* tagsGroup = new QGroupBox(tr("标签"));
    QVBoxLayout* tagsLayout = new QVBoxLayout(tagsGroup);
    m_totalTagsLabel = new QLabel;
    m_totalTagsLabel->setStyleSheet("font-size: 32px; font-weight: bold;");
    m_totalTagsLabel->setAlignment(Qt::AlignCenter);
    tagsLayout->addWidget(m_totalTagsLabel);
    tagsLayout->addWidget(new QLabel(tr("标签总数")));
    statsGrid->addWidget(tagsGroup, 0, 2);

    QGroupBox* favoritesGroup = new QGroupBox(tr("收藏"));
    QVBoxLayout* favoritesLayout = new QVBoxLayout(favoritesGroup);
    m_favoriteCountLabel = new QLabel;
    m_favoriteCountLabel->setStyleSheet("font-size: 32px; font-weight: bold;");
    m_favoriteCountLabel->setAlignment(Qt::AlignCenter);
    favoritesLayout->addWidget(m_favoriteCountLabel);
    favoritesLayout->addWidget(new QLabel(tr("收藏提示词")));
    statsGrid->addWidget(favoritesGroup, 1, 0);

    QGroupBox* ratingGroup = new QGroupBox(tr("平均评分"));
    QVBoxLayout* ratingLayout = new QVBoxLayout(ratingGroup);
    m_avgRatingLabel = new QLabel;
    m_avgRatingLabel->setStyleSheet("font-size: 32px; font-weight: bold;");
    m_avgRatingLabel->setAlignment(Qt::AlignCenter);
    ratingLayout->addWidget(m_avgRatingLabel);
    ratingLayout->addWidget(new QLabel(tr("整体平均")));
    statsGrid->addWidget(ratingGroup, 1, 1);

    mainLayout->addLayout(statsGrid);
    mainLayout->addStretch();

    mainLayout->addWidget(new QLabel(tr("快捷提示：")));
    QString tipsText = 
        tr("点击[新建]创建新提示词\n") +
        tr("双击提示词进行编辑\n") +
        tr("使用搜索框查找提示词\n") +
        tr("在左侧面板按分类或标签筛选\n") +
        tr("使用星级评分对提示词评分");
    QLabel* tipsLabel = new QLabel(tipsText);
    tipsLabel->setStyleSheet("color: #6c757d; font-size: 12px;");
    mainLayout->addWidget(tipsLabel);
}

void DashboardWidget::updateStats()
{
    int totalPrompts = m_promptService->getPromptCount();
    int totalCategories = m_categoryService->getAllCategories().size();
    int totalTags = m_tagService->getAllTags().size();
    int favorites = m_promptService->findFavorites().size();
    double avgRating = m_promptService->getAverageRating();

    m_totalPromptsLabel->setNum(totalPrompts);
    m_totalCategoriesLabel->setNum(totalCategories);
    m_totalTagsLabel->setNum(totalTags);
    m_favoriteCountLabel->setNum(favorites);
    m_avgRatingLabel->setText(QString::number(avgRating, 'f', 1));
}

void DashboardWidget::refresh()
{
    updateStats();
}
