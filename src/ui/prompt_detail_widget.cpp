#include "prompt_detail_widget.h"
#include "prompt_service.h"
#include "tag_service.h"
#include "category_service.h"
#include "star_rating.h"
#include "repository/model_score_repository.h"

#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QClipboard>
#include <QApplication>
#include <QDateTime>
#include <QTimer>
#include <QFrame>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QScrollArea>
#include <QToolButton>

PromptDetailWidget::PromptDetailWidget(PromptService* promptService,
                                         TagService* tagService,
                                         CategoryService* categoryService,
                                         QWidget* parent)
    : QWidget(parent)
    , m_promptService(promptService)
    , m_tagService(tagService)
    , m_categoryService(categoryService)
{
    setupUi();
}

void PromptDetailWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto createCard = [](const QString& style = QString()) {
        QFrame* card = new QFrame;
        card->setFrameShape(QFrame::NoFrame);
        card->setStyleSheet(
            "QFrame { background: #ffffff; border: 1px solid #e8edf3; border-radius: 12px; }" + style);
        return card;
    };

    auto createSectionToggle = [](const QString& text) {
        QToolButton* button = new QToolButton;
        button->setText(text);
        button->setCheckable(true);
        button->setChecked(false);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setArrowType(Qt::RightArrow);
        button->setStyleSheet(
            "QToolButton { border: none; color: #1f2937; font-size: 13px; font-weight: 600; padding: 0; }"
            "QToolButton::menu-indicator { image: none; }");
        return button;
    };

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: #f6f8fb; border: none; }");

    QWidget* contentWidget = new QWidget;
    contentWidget->setStyleSheet("QWidget { background: #f6f8fb; }");
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(14);

    QFrame* headerCard = createCard();
    QVBoxLayout* headerCardLayout = new QVBoxLayout(headerCard);
    headerCardLayout->setContentsMargins(18, 18, 18, 18);
    headerCardLayout->setSpacing(12);

    m_headerWidget = new QWidget;
    QHBoxLayout* headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(16);

    m_titleLabel = new QLabel;
    m_titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #111827;");
    m_titleLabel->setWordWrap(true);
    headerLayout->addWidget(m_titleLabel, 1);

    m_ratingLabel = new QLabel;
    m_ratingLabel->setStyleSheet("color: #f59e0b; font-size: 16px; font-weight: 600; background: #fff7e6; border-radius: 999px; padding: 6px 10px;");
    headerLayout->addWidget(m_ratingLabel);

    m_copyBtn = new QPushButton(tr("复制"));
    m_copyBtn->setFixedHeight(36);
    m_copyBtn->setCursor(Qt::PointingHandCursor);
    m_copyBtn->setStyleSheet(
        "QPushButton { background: #1677ff; color: #fff; border: none; border-radius: 8px; padding: 6px 16px; font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: #4096ff; }"
        "QPushButton:pressed { background: #096dd9; }"
    );
    headerLayout->addWidget(m_copyBtn);
    connect(m_copyBtn, &QPushButton::clicked, this, &PromptDetailWidget::onCopyContent);

    headerCardLayout->addWidget(m_headerWidget);

    m_metaLabel = new QLabel;
    m_metaLabel->setStyleSheet("color: #4b5563; font-size: 12px; line-height: 1.5;");
    m_metaLabel->setWordWrap(true);
    headerCardLayout->addWidget(m_metaLabel);

    m_timeLabel = new QLabel;
    m_timeLabel->setStyleSheet("color: #9ca3af; font-size: 11px;");
    m_timeLabel->setWordWrap(true);
    headerCardLayout->addWidget(m_timeLabel);

    QFrame* ratingCard = createCard();
    QVBoxLayout* ratingCardLayout = new QVBoxLayout(ratingCard);
    ratingCardLayout->setContentsMargins(18, 16, 18, 16);
    ratingCardLayout->setSpacing(10);

    QLabel* ratingCardTitle = new QLabel(tr("评分概览"));
    ratingCardTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #111827;");
    ratingCardLayout->addWidget(ratingCardTitle);

    QHBoxLayout* ratingLayout = new QHBoxLayout;
    ratingLayout->setSpacing(8);

    QLabel* ratingTitle = new QLabel(tr("总体评分"));
    ratingTitle->setStyleSheet("color: #4b5563; font-size: 13px;");
    ratingLayout->addWidget(ratingTitle);

    m_ratingWidget = new StarRating;
    m_ratingWidget->setStarSize(20);
    connect(m_ratingWidget, &StarRating::editingFinished, this, [this]() {
        onRatingChanged(m_ratingWidget->rating());
    });
    ratingLayout->addWidget(m_ratingWidget);
    ratingLayout->addStretch();
    ratingCardLayout->addLayout(ratingLayout);

    m_modelScoresSummaryLabel = new QLabel;
    m_modelScoresSummaryLabel->setStyleSheet("color: #6b7280; font-size: 12px;");
    m_modelScoresSummaryLabel->setWordWrap(true);
    ratingCardLayout->addWidget(m_modelScoresSummaryLabel);

    QFrame* contentCard = createCard();
    QVBoxLayout* contentCardLayout = new QVBoxLayout(contentCard);
    contentCardLayout->setContentsMargins(18, 18, 18, 18);
    contentCardLayout->setSpacing(12);

    QLabel* contentTitle = new QLabel(tr("提示词内容"));
    contentTitle->setStyleSheet("font-weight: 600; color: #111827; font-size: 13px;");
    contentCardLayout->addWidget(contentTitle);

    m_contentEdit = new QTextEdit;
    m_contentEdit->setReadOnly(true);
    m_contentEdit->setStyleSheet(
        "QTextEdit { background: #fbfcfe; border: 1px solid #e5e7eb; border-radius: 10px; padding: 14px; "
        "font-family: 'Consolas', 'Monaco', 'Courier New', monospace; font-size: 13px; line-height: 1.6; color: #111827; }"
    );
    m_contentEdit->setMinimumHeight(200);
    contentCardLayout->addWidget(m_contentEdit);

    QFrame* descriptionCard = createCard();
    m_descCard = descriptionCard;
    QVBoxLayout* descriptionCardLayout = new QVBoxLayout(descriptionCard);
    descriptionCardLayout->setContentsMargins(18, 16, 18, 16);
    descriptionCardLayout->setSpacing(12);

    m_descToggle = createSectionToggle(tr("描述与补充说明"));
    connect(m_descToggle, &QToolButton::toggled, this, &PromptDetailWidget::setDescriptionExpanded);
    descriptionCardLayout->addWidget(m_descToggle);

    m_descSection = new QWidget;
    QVBoxLayout* descSectionLayout = new QVBoxLayout(m_descSection);
    descSectionLayout->setContentsMargins(0, 0, 0, 0);
    descSectionLayout->setSpacing(0);
    m_descLabel = new QLabel;
    m_descLabel->setStyleSheet("color: #4b5563; font-size: 12px; line-height: 1.5; background: #fbfcfe; border: 1px solid #e5e7eb; border-radius: 10px; padding: 12px;");
    m_descLabel->setWordWrap(true);
    descSectionLayout->addWidget(m_descLabel);
    descriptionCardLayout->addWidget(m_descSection);

    QFrame* scoresCard = createCard();
    QVBoxLayout* scoresCardLayout = new QVBoxLayout(scoresCard);
    scoresCardLayout->setContentsMargins(18, 16, 18, 16);
    scoresCardLayout->setSpacing(12);

    QHBoxLayout* scoreHeaderLayout = new QHBoxLayout;
    m_modelScoresToggle = createSectionToggle(tr("模型评分详情"));
    connect(m_modelScoresToggle, &QToolButton::toggled, this, &PromptDetailWidget::setModelScoresExpanded);
    scoreHeaderLayout->addWidget(m_modelScoresToggle);
    scoreHeaderLayout->addStretch();

    m_addScoreBtn = new QPushButton(tr("+ 添加评分"));
    m_addScoreBtn->setFixedHeight(32);
    m_addScoreBtn->setCursor(Qt::PointingHandCursor);
    m_addScoreBtn->setStyleSheet(
        "QPushButton { background: #52c41a; color: #fff; border: none; border-radius: 8px; padding: 4px 14px; font-size: 12px; font-weight: 600; }"
        "QPushButton:hover { background: #73d13d; }"
    );
    scoreHeaderLayout->addWidget(m_addScoreBtn);
    connect(m_addScoreBtn, &QPushButton::clicked, this, &PromptDetailWidget::onAddModelScore);
    scoresCardLayout->addLayout(scoreHeaderLayout);

    m_modelScoresSection = new QWidget;
    QVBoxLayout* scoresSectionLayout = new QVBoxLayout(m_modelScoresSection);
    scoresSectionLayout->setContentsMargins(0, 0, 0, 0);
    scoresSectionLayout->setSpacing(0);

    m_modelScoresTable = new QTableWidget;
    m_modelScoresTable->setColumnCount(4);
    m_modelScoresTable->setHorizontalHeaderLabels({tr("模型"), tr("评分"), tr("备注"), tr("操作")});
    m_modelScoresTable->horizontalHeader()->setStretchLastSection(true);
    m_modelScoresTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_modelScoresTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_modelScoresTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_modelScoresTable->setAlternatingRowColors(true);
    m_modelScoresTable->verticalHeader()->setVisible(false);
    m_modelScoresTable->setFocusPolicy(Qt::NoFocus);
    m_modelScoresTable->setStyleSheet(
        "QTableWidget { background: #fff; border: 1px solid #e8e8e8; border-radius: 6px; }"
        "QTableWidget::item { padding: 8px; }"
        "QTableWidget::item:selected { background: #fff; color: #333; }"
        "QHeaderView::section { background: #fafafa; padding: 8px; border: none; border-bottom: 1px solid #e8e8e8; font-weight: 600; }"
    );
    m_modelScoresTable->setColumnWidth(0, 120);
    m_modelScoresTable->setColumnWidth(1, 100);
    m_modelScoresTable->setColumnWidth(2, 200);
    m_modelScoresTable->setMinimumHeight(88);
    m_modelScoresTable->setMaximumHeight(232);
    scoresSectionLayout->addWidget(m_modelScoresTable);
    scoresCardLayout->addWidget(m_modelScoresSection);

    contentLayout->addWidget(headerCard);
    contentLayout->addWidget(contentCard, 1);
    contentLayout->addWidget(ratingCard);
    contentLayout->addWidget(descriptionCard);
    contentLayout->addWidget(scoresCard);
    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    setDescriptionExpanded(false);
    setModelScoresExpanded(false);

    clear();
}

void PromptDetailWidget::setPrompt(int promptId)
{
    m_currentPromptId = promptId;
    updateDisplay();
}

void PromptDetailWidget::clear()
{
    m_currentPromptId = 0;
    m_titleLabel->setText(tr("未选择提示词"));
    m_ratingLabel->clear();
    m_metaLabel->clear();
    m_timeLabel->clear();
    m_contentEdit->clear();
    m_descLabel->clear();
    m_ratingWidget->setRating(0);
    m_modelScoresTable->setRowCount(0);
    m_modelScoresSummaryLabel->setText(tr("暂无模型评分"));
    m_descCard->setVisible(false);
    m_descToggle->setVisible(false);
    setDescriptionExpanded(false);
    setModelScoresExpanded(false);
}

void PromptDetailWidget::updateDisplay()
{
    if (m_currentPromptId <= 0) {
        clear();
        return;
    }

    Prompt prompt = m_promptService->getPrompt(m_currentPromptId);
    if (!prompt.isValid()) {
        clear();
        return;
    }

    m_titleLabel->setText(prompt.title);
    m_ratingLabel->setText(formatStars(prompt.overallRating));

    QStringList metaParts;
    if (prompt.categoryId > 0) {
        Category cat = m_categoryService->getCategory(prompt.categoryId);
        if (cat.isValid()) {
            metaParts << tr("分类：%1").arg(cat.name);
        }
    }

    QStringList tagNames;
    for (int tagId : prompt.tagIds) {
        Tag tag = m_tagService->getTag(tagId);
        if (tag.isValid()) {
            tagNames << tag.name;
        }
    }
    if (!tagNames.isEmpty()) {
        metaParts << tr("标签：%1").arg(tagNames.join("、"));
    }
    m_metaLabel->setText(metaParts.join("  |  "));
    m_timeLabel->setText(tr("更新于 %1  |  使用次数 %2")
        .arg(prompt.updatedAt.toString("yyyy-MM-dd"))
        .arg(prompt.usageCount));

    m_contentEdit->setPlainText(prompt.content);
    const bool hasDescription = !prompt.description.trimmed().isEmpty();
    m_descLabel->setText(prompt.description);
    m_descCard->setVisible(hasDescription);
    m_descToggle->setVisible(hasDescription);
    const bool shouldExpandDescription = hasDescription && prompt.description.trimmed().size() <= 120;
    setDescriptionExpanded(shouldExpandDescription);
    m_ratingWidget->setRating(prompt.overallRating);

    loadModelScores();
}

void PromptDetailWidget::loadModelScores()
{
    m_modelScoresTable->setRowCount(0);

    if (m_currentPromptId <= 0) {
        m_modelScoresSummaryLabel->setText(tr("暂无模型评分"));
        return;
    }

    ModelScoreRepository scoreRepo;
    QVector<ModelScore> scores = scoreRepo.findByPromptId(m_currentPromptId);
    if (scores.isEmpty()) {
        m_modelScoresSummaryLabel->setText(tr("暂无模型评分，点击“添加评分”开始记录模型表现。"));
        setModelScoresExpanded(false);
    } else {
        double totalScore = 0.0;
        for (const ModelScore& score : scores) {
            totalScore += score.score;
        }
        m_modelScoresSummaryLabel->setText(
            tr("已记录 %1 个模型评分，平均分 %2")
                .arg(scores.size())
                .arg(QString::number(totalScore / scores.size(), 'f', 1)));
        setModelScoresExpanded(scores.size() <= 2);
    }

    const int visibleRows = qMin(scores.size(), 4);
    const int tableHeight = 40 + visibleRows * 44 + (visibleRows > 0 ? 2 : 0);
    m_modelScoresTable->setFixedHeight(qBound(88, tableHeight, 232));

    m_modelScoresTable->setRowCount(scores.size());
    for (int i = 0; i < scores.size(); ++i) {
        const ModelScore& score = scores[i];
        auto model = scoreRepo.findModelById(score.modelId);
        QString modelName = model ? model->name : tr("未知");
        m_modelScoresTable->setRowHeight(i, 44);

        QTableWidgetItem* modelItem = new QTableWidgetItem(modelName);
        modelItem->setFlags(modelItem->flags() & ~Qt::ItemIsEditable);
        modelItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_modelScoresTable->setItem(i, 0, modelItem);

        QTableWidgetItem* ratingItem = new QTableWidgetItem(formatStars(score.score));
        ratingItem->setFlags(ratingItem->flags() & ~Qt::ItemIsEditable);
        ratingItem->setForeground(QColor("#f59e0b"));
        ratingItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_modelScoresTable->setItem(i, 1, ratingItem);

        QTableWidgetItem* noteItem = new QTableWidgetItem(score.note);
        noteItem->setFlags(noteItem->flags() & ~Qt::ItemIsEditable);
        noteItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_modelScoresTable->setItem(i, 2, noteItem);

        QWidget* btnWidget = new QWidget;
        QHBoxLayout* btnLayout = new QHBoxLayout(btnWidget);
        btnLayout->setContentsMargins(8, 4, 8, 4);
        btnLayout->setSpacing(4);
        btnLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        QPushButton* editBtn = new QPushButton(tr("编辑"));
        editBtn->setFixedSize(56, 28);
        editBtn->setStyleSheet("QPushButton { background: #1890ff; color: #fff; border: none; border-radius: 3px; font-size: 11px; } QPushButton:hover { background: #40a9ff; }");
        editBtn->setProperty("scoreId", score.id);
        connect(editBtn, &QPushButton::clicked, this, &PromptDetailWidget::onEditModelScore);
        btnLayout->addWidget(editBtn);

        QPushButton* deleteBtn = new QPushButton(tr("删除"));
        deleteBtn->setFixedSize(56, 28);
        deleteBtn->setStyleSheet("QPushButton { background: #ff4d4f; color: #fff; border: none; border-radius: 3px; font-size: 11px; } QPushButton:hover { background: #ff7875; }");
        deleteBtn->setProperty("scoreId", score.id);
        connect(deleteBtn, &QPushButton::clicked, this, &PromptDetailWidget::onDeleteModelScore);
        btnLayout->addWidget(deleteBtn);

        btnLayout->addStretch();
        m_modelScoresTable->setCellWidget(i, 3, btnWidget);
    }
}

QString PromptDetailWidget::formatStars(double rating)
{
    return StarRatingText::toDisplayString(rating);
}

void PromptDetailWidget::setDescriptionExpanded(bool expanded)
{
    m_descSection->setVisible(expanded && m_descToggle->isVisible());
    m_descToggle->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
    if (m_descToggle->isChecked() != expanded) {
        m_descToggle->setChecked(expanded);
    }
}

void PromptDetailWidget::setModelScoresExpanded(bool expanded)
{
    m_modelScoresSection->setVisible(expanded);
    m_modelScoresToggle->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
    if (m_modelScoresToggle->isChecked() != expanded) {
        m_modelScoresToggle->setChecked(expanded);
    }
}

void PromptDetailWidget::onCopyContent()
{
    if (m_currentPromptId <= 0) {
        return;
    }

    m_promptService->copyToClipboard(m_currentPromptId);
    m_copyBtn->setText(tr("已复制！"));
    QTimer::singleShot(2000, [this]() {
        m_copyBtn->setText(tr("复制"));
    });
}

void PromptDetailWidget::onRatingChanged(double rating)
{
    if (m_currentPromptId <= 0) {
        return;
    }

    m_promptService->updateRating(m_currentPromptId, rating);
    m_ratingLabel->setText(formatStars(rating));
}

void PromptDetailWidget::onAddModelScore()
{
    if (m_currentPromptId <= 0) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择一个提示词"));
        return;
    }

    ModelScoreRepository scoreRepo;
    QVector<Model> models = scoreRepo.findAllModels();

    if (models.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("没有可用的模型，请先在数据库中添加模型"));
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("添加模型评分"));
    dialog.setMinimumWidth(400);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(16);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setSpacing(12);

    QComboBox* modelCombo = new QComboBox;
    modelCombo->setMinimumWidth(200);
    for (const Model& model : models) {
        modelCombo->addItem(model.name, model.id);
    }
    formLayout->addRow(tr("选择模型："), modelCombo);

    QWidget* ratingWidget = new QWidget;
    QHBoxLayout* ratingLayout = new QHBoxLayout(ratingWidget);
    ratingLayout->setContentsMargins(0, 0, 0, 0);
    
    StarRating* starRating = new StarRating;
    starRating->setStarSize(28);
    ratingLayout->addWidget(starRating);
    ratingLayout->addStretch();
    formLayout->addRow(tr("评分："), ratingWidget);

    QLineEdit* noteEdit = new QLineEdit;
    noteEdit->setPlaceholderText(tr("输入备注信息（可选）"));
    noteEdit->setMinimumWidth(200);
    formLayout->addRow(tr("备注："), noteEdit);

    mainLayout->addLayout(formLayout);

    mainLayout->addSpacing(8);

    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    QPushButton* saveBtn = buttonBox->addButton(tr("保存"), QDialogButtonBox::AcceptRole);
    saveBtn->setStyleSheet("QPushButton { background: #1890ff; color: #fff; border: none; border-radius: 4px; padding: 8px 24px; min-width: 80px; } QPushButton:hover { background: #40a9ff; }");
    QPushButton* cancelBtn = buttonBox->addButton(tr("取消"), QDialogButtonBox::RejectRole);
    cancelBtn->setStyleSheet("QPushButton { background: #f5f5f5; color: #333; border: 1px solid #d9d9d9; border-radius: 4px; padding: 8px 24px; min-width: 80px; } QPushButton:hover { background: #fafafa; }");
    buttonBox->setCenterButtons(true);
    
    connect(saveBtn, &QPushButton::clicked, [&]() {
        if (modelCombo->currentIndex() < 0) {
            QMessageBox::warning(&dialog, tr("提示"), tr("请选择一个模型"));
            return;
        }
        if (starRating->rating() <= 0) {
            QMessageBox::warning(&dialog, tr("提示"), tr("请设置评分"));
            return;
        }
        dialog.accept();
    });
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    mainLayout->addWidget(buttonBox);

    if (dialog.exec() == QDialog::Accepted) {
        ModelScore score;
        score.promptId = m_currentPromptId;
        score.modelId = modelCombo->currentData().toInt();
        score.score = starRating->rating();
        score.note = noteEdit->text().trimmed();

        int id = scoreRepo.insertScore(score);
        if (id > 0) {
            loadModelScores();
            QMessageBox::information(this, tr("成功"), tr("评分添加成功"));
        } else {
            QMessageBox::warning(this, tr("错误"), tr("添加评分失败"));
        }
    }
}

void PromptDetailWidget::onDeleteModelScore()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int scoreId = btn->property("scoreId").toInt();
    if (scoreId <= 0) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("确认删除"), tr("确定要删除这条评分吗？"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        ModelScoreRepository scoreRepo;
        scoreRepo.removeScore(scoreId);
        loadModelScores();
    }
}

void PromptDetailWidget::onEditModelScore()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int scoreId = btn->property("scoreId").toInt();
    if (scoreId <= 0) return;

    ModelScoreRepository scoreRepo;
    auto scores = scoreRepo.findByPromptId(m_currentPromptId);
    ModelScore currentScore;
    for (const auto& s : scores) {
        if (s.id == scoreId) {
            currentScore = s;
            break;
        }
    }

    if (currentScore.id <= 0) return;

    QVector<Model> models = scoreRepo.findAllModels();

    QDialog dialog(this);
    dialog.setWindowTitle(tr("编辑模型评分"));
    dialog.setMinimumWidth(400);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(16);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setSpacing(12);

    QComboBox* modelCombo = new QComboBox;
    modelCombo->setMinimumWidth(200);
    for (const Model& model : models) {
        modelCombo->addItem(model.name, model.id);
        if (model.id == currentScore.modelId) {
            modelCombo->setCurrentIndex(modelCombo->count() - 1);
        }
    }
    formLayout->addRow(tr("选择模型："), modelCombo);

    QWidget* ratingWidget = new QWidget;
    QHBoxLayout* ratingLayout = new QHBoxLayout(ratingWidget);
    ratingLayout->setContentsMargins(0, 0, 0, 0);
    
    StarRating* starRating = new StarRating;
    starRating->setStarSize(28);
    starRating->setRating(currentScore.score);
    ratingLayout->addWidget(starRating);
    ratingLayout->addStretch();
    formLayout->addRow(tr("评分："), ratingWidget);

    QLineEdit* noteEdit = new QLineEdit;
    noteEdit->setText(currentScore.note);
    noteEdit->setMinimumWidth(200);
    formLayout->addRow(tr("备注："), noteEdit);

    mainLayout->addLayout(formLayout);

    mainLayout->addSpacing(8);

    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    QPushButton* saveBtn = buttonBox->addButton(tr("保存"), QDialogButtonBox::AcceptRole);
    saveBtn->setStyleSheet("QPushButton { background: #1890ff; color: #fff; border: none; border-radius: 4px; padding: 8px 24px; min-width: 80px; } QPushButton:hover { background: #40a9ff; }");
    QPushButton* cancelBtn = buttonBox->addButton(tr("取消"), QDialogButtonBox::RejectRole);
    cancelBtn->setStyleSheet("QPushButton { background: #f5f5f5; color: #333; border: 1px solid #d9d9d9; border-radius: 4px; padding: 8px 24px; min-width: 80px; } QPushButton:hover { background: #fafafa; }");
    buttonBox->setCenterButtons(true);
    
    connect(saveBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    mainLayout->addWidget(buttonBox);

    if (dialog.exec() == QDialog::Accepted) {
        currentScore.modelId = modelCombo->currentData().toInt();
        currentScore.score = starRating->rating();
        currentScore.note = noteEdit->text().trimmed();

        if (scoreRepo.updateScore(currentScore)) {
            loadModelScores();
            QMessageBox::information(this, tr("成功"), tr("评分更新成功"));
        }
    }
}
