#pragma once

#include <QWidget>
#include <QTableWidget>

class PromptService;
class TagService;
class CategoryService;
class QLabel;
class QTextEdit;
class QPushButton;
class StarRating;
class QComboBox;
class QToolButton;

class PromptDetailWidget : public QWidget {
    Q_OBJECT

public:
    explicit PromptDetailWidget(PromptService* promptService,
                                 TagService* tagService,
                                 CategoryService* categoryService,
                                 QWidget* parent = nullptr);

    void setPrompt(int promptId);
    void clear();

private slots:
    void onCopyContent();
    void onRatingChanged(double rating);
    void onAddModelScore();
    void onDeleteModelScore();
    void onEditModelScore();

private:
    void setupUi();
    void updateDisplay();
    void loadModelScores();
    QString formatStars(double rating);
    void setDescriptionExpanded(bool expanded);
    void setModelScoresExpanded(bool expanded);

    PromptService* m_promptService;
    TagService* m_tagService;
    CategoryService* m_categoryService;

    int m_currentPromptId = 0;

    QWidget* m_headerWidget;
    QLabel* m_titleLabel;
    QLabel* m_ratingLabel;
    QPushButton* m_copyBtn;

    QLabel* m_metaLabel;
    QLabel* m_timeLabel;
    QTextEdit* m_contentEdit;
    QWidget* m_descCard;
    QLabel* m_descLabel;
    QToolButton* m_descToggle;
    QWidget* m_descSection;
    QTableWidget* m_modelScoresTable;
    QPushButton* m_addScoreBtn;
    QLabel* m_modelScoresSummaryLabel;
    QToolButton* m_modelScoresToggle;
    QWidget* m_modelScoresSection;

    StarRating* m_ratingWidget;
};
