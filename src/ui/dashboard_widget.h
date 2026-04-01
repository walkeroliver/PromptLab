#pragma once

#include <QWidget>

class PromptService;
class CategoryService;
class TagService;
class QLabel;

class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWidget(PromptService* promptService,
                              CategoryService* categoryService,
                              TagService* tagService,
                              QWidget* parent = nullptr);

    void refresh();

private:
    void setupUi();
    void updateStats();

    PromptService* m_promptService;
    CategoryService* m_categoryService;
    TagService* m_tagService;

    QLabel* m_totalPromptsLabel;
    QLabel* m_totalCategoriesLabel;
    QLabel* m_totalTagsLabel;
    QLabel* m_favoriteCountLabel;
    QLabel* m_avgRatingLabel;
};
